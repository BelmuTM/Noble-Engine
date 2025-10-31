#include "VulkanShaderProgram.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "core/debug/ErrorHandling.h"
#include "core/ResourceManager.h"

#include <fstream>
#include <ranges>
#include <unordered_map>

static const std::unordered_map<std::string, std::pair<vk::ShaderStageFlagBits, const char*>> stageData = {
    {"vert", {vk::ShaderStageFlagBits::eVertex,   "vertMain"}},
    {"frag", {vk::ShaderStageFlagBits::eFragment, "fragMain"}},
    {"comp", {vk::ShaderStageFlagBits::eCompute,  "compMain"}}
};

VulkanShaderProgram::~VulkanShaderProgram() {
    clearShaderModules();
}

void VulkanShaderProgram::clearShaderModules() {
    for (const auto& module : shaderModules) {
        _device.destroyShaderModule(module);
    }
    shaderModules.clear();
}

bool VulkanShaderProgram::loadFromFiles(const std::vector<std::string>& paths, std::string& errorMessage) {
    if (paths.empty()) {
        errorMessage = "Failed to load shader program: no paths provided";
        return false;
    }

    ScopeGuard guard{[this] { clearShaderModules(); }};

    for (const auto& path : paths) {
        errorMessage = "Failed to load shader stage \"" + path + "\": ";

        std::string stageExtension = extractStageExtension(path);

        const auto it = stageData.find(stageExtension);

        if (stageExtension.empty() || it == stageData.end()) {
            errorMessage += "incorrect file extension \"" + stageExtension + "\"";
            return false;
        }

        const auto [stageFlag, entryPoint] = it->second;

        const std::vector<char>& bytecode = readShaderFile(shaderFilesPath + path);
        if (bytecode.empty()) {
            errorMessage += "bytecode is empty (file does not exist or is zero bytes)";
            return {};
        }

        const vk::ShaderModule& module = createShaderModule(bytecode, errorMessage);
        if (!module) return false;

        shaderModules.push_back(module);

        vk::PipelineShaderStageCreateInfo stageInfo{};
        stageInfo
            .setStage(stageFlag)
            .setModule(module)
            .setPName(entryPoint);

        shaderStages.push_back(stageInfo);
    }

    guard.release();

    return true;
}

bool VulkanShaderProgram::load(const std::string& name, std::string& errorMessage) {
    return loadFromFiles(findShaderFilePaths(name), errorMessage);
}

std::string VulkanShaderProgram::extractStageExtension(const std::string& path) noexcept {
    const auto lastDot       = path.rfind('.');
    const auto secondLastDot = path.rfind('.', lastDot - 1);
    if (lastDot == std::string::npos || secondLastDot == std::string::npos) {
        return "";
    }
    return path.substr(secondLastDot + 1, lastDot - secondLastDot - 1);
}

std::vector<char> VulkanShaderProgram::readShaderFile(const std::string& path) noexcept {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        return {};
    }

    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

    file.close();

    return buffer;
}

vk::ShaderModule VulkanShaderProgram::createShaderModule(
    const std::vector<char>& code, std::string& errorMessage
) const {
    vk::ShaderModuleCreateInfo shaderModuleInfo{};
    shaderModuleInfo
        .setCodeSize(sizeof(char) * code.size())
        .setPCode(reinterpret_cast<const uint32_t*>(code.data()));

    const auto shaderModuleCreate = VK_CALL(_device.createShaderModule(shaderModuleInfo), errorMessage);
    if (shaderModuleCreate.result != vk::Result::eSuccess) {
        return {};
    }

    return shaderModuleCreate.value;
}

std::vector<std::string> VulkanShaderProgram::findShaderFilePaths(const std::string& name) {
    std::vector<std::string> paths{};
    for (const auto& stageExtension : stageData | std::views::keys) {
        const std::string relativePath = name + "." + stageExtension + ".spv";
        const std::string fullPath     = shaderFilesPath + relativePath;

        if (FILE* file = fopen(fullPath.c_str(), "r")) {
            fclose(file);
            paths.push_back(relativePath);
        }
    }
    return paths;
}
