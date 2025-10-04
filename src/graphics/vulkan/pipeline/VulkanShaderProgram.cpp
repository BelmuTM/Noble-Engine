#include "VulkanShaderProgram.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

#include <fstream>
#include <unordered_map>

static const std::string shaderFilesPath = "../../shaders/spv/";

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

bool VulkanShaderProgram::loadFromFiles(const std::vector<std::string>& shaderPaths, std::string& errorMessage) {
    for (const auto& path : shaderPaths) {
        errorMessage = "Failed to load shader stage \"" + path + "\": ";

        std::string stageExtension = extractStageExtension(path);

        const auto it = stageData.find(stageExtension);

        if (stageExtension.empty() || it == stageData.end()) {
            errorMessage += "incorrect file extension \"" + stageExtension + "\"";
            clearShaderModules();
            return false;
        }

        const auto [stageFlag, entryPoint] = it->second;

        const std::vector<char>& bytecode = readShaderFile(shaderFilesPath + path);
        if (bytecode.empty()) {
            errorMessage += "bytecode is empty (file does not exist or is zero bytes)";
            clearShaderModules();
            return {};
        }

        const vk::ShaderModule& module = createShaderModule(bytecode, errorMessage);
        if (!module) {
            clearShaderModules();
            return false;
        }

        shaderModules.push_back(module);

        vk::PipelineShaderStageCreateInfo stageInfo{};
        stageInfo
            .setStage(stageFlag)
            .setModule(module)
            .setPName(entryPoint);

        shaderStages.push_back(stageInfo);
    }
    return true;
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
