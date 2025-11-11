#include "VulkanShaderProgram.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "core/ResourceManager.h"
#include "core/debug/ErrorHandling.h"

#include <fstream>
#include <ranges>
#include <unordered_map>

#include <spirv_cross/spirv_cross.hpp>

static const std::unordered_map<std::string, std::pair<vk::ShaderStageFlagBits, const char*>> stageData = {
    {"vert", {vk::ShaderStageFlagBits::eVertex,   "vertMain"}},
    {"frag", {vk::ShaderStageFlagBits::eFragment, "fragMain"}},
    {"comp", {vk::ShaderStageFlagBits::eCompute,  "compMain"}}
};

VulkanShaderProgram::~VulkanShaderProgram() {
    clearShaderModules();
}

void VulkanShaderProgram::clearShaderModules() {
    for (const auto& module : _shaderModules) {
        _device.destroyShaderModule(module);
    }
    _shaderModules.clear();
}

vk::ShaderModule VulkanShaderProgram::createShaderModule(
    const std::vector<uint32_t>& bytecode, std::string& errorMessage
) const {
    vk::ShaderModuleCreateInfo shaderModuleInfo{};
    shaderModuleInfo
        .setCodeSize(sizeof(uint32_t) * bytecode.size())
        .setPCode(bytecode.data());

    const auto shaderModuleCreate = VK_CALL(_device.createShaderModule(shaderModuleInfo), errorMessage);
    if (shaderModuleCreate.result != vk::Result::eSuccess) {
        return {};
    }

    return shaderModuleCreate.value;
}

bool VulkanShaderProgram::loadFromFiles(
    const std::vector<std::string>& paths, const bool fullscreen, std::string& errorMessage
) {
    if (paths.empty()) {
        errorMessage = "Failed to load shader program: no paths provided";
        return false;
    }

    _isFullscreen = fullscreen;

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

        const std::vector<uint32_t>& bytecode = readShaderSPIRVBytecode(shaderFilesPath + path);
        if (bytecode.empty()) {
            errorMessage += "bytecode is empty (file does not exist or is zero bytes)";
            return {};
        }

        const vk::ShaderModule& module = createShaderModule(bytecode, errorMessage);
        if (!module) return false;

        _shaderModules.push_back(module);

        vk::PipelineShaderStageCreateInfo stageInfo{};
        stageInfo
            .setStage(stageFlag)
            .setModule(module)
            .setPName(entryPoint);

        _shaderStages.push_back(stageInfo);
        _stageFlags |= stageFlag;

        //TRY(reflectDescriptors(bytecode, stageFlag, errorMessage));
    }

    guard.release();

    return true;
}

bool VulkanShaderProgram::load(const std::string& name, const bool fullscreen, std::string& errorMessage) {
    return loadFromFiles(findShaderFilePaths(name), fullscreen, errorMessage);
}

/*
bool VulkanShaderProgram::reflectDescriptors(
    const std::vector<uint32_t>& bytecode, const vk::ShaderStageFlags stageFlag, std::string& errorMessage
) {
    const spirv_cross::Compiler compiler(bytecode);

    const spirv_cross::ShaderResources resources = compiler.get_shader_resources();

    for (const auto& uniformBuffer : resources.uniform_buffers) {
        DescriptorBindingInfo info{};
        info.set        = compiler.get_decoration(uniformBuffer.id, spv::DecorationDescriptorSet);
        info.binding    = compiler.get_decoration(uniformBuffer.id, spv::DecorationBinding);
        info.type       = vk::DescriptorType::eUniformBuffer;
        info.count      = 1;
        info.stageFlags = stageFlag;
        _bindings.push_back(info);
    }

    for (const auto& imageSampler : resources.sampled_images) {
        DescriptorBindingInfo info{};
        info.set        = compiler.get_decoration(imageSampler.id, spv::DecorationDescriptorSet);
        info.binding    = compiler.get_decoration(imageSampler.id, spv::DecorationBinding);
        info.type       = vk::DescriptorType::eCombinedImageSampler;
        info.count      = 1;
        info.stageFlags = stageFlag;
        _bindings.push_back(info);
    }

    std::unordered_map<uint32_t, std::vector<vk::DescriptorSetLayoutBinding>> setBindings;

    for (const auto& [set, binding, type, count, stageFlags] : _bindings) {
        setBindings[set].emplace_back(binding, type, count, stageFlags, nullptr);
    }

    for (const auto& bindings : setBindings | std::views::values) {
        vk::DescriptorSetLayout layout;
        TRY(_descriptorManager->createSetLayout(bindings, errorMessage));

        _descriptorSetLayouts.push_back(layout);
    }

    return true;
}
*/

std::string VulkanShaderProgram::extractStageExtension(const std::string& path) noexcept {
    const auto lastDot       = path.rfind('.');
    const auto secondLastDot = path.rfind('.', lastDot - 1);
    if (lastDot == std::string::npos || secondLastDot == std::string::npos) {
        return "";
    }
    return path.substr(secondLastDot + 1, lastDot - secondLastDot - 1);
}

std::vector<uint32_t> VulkanShaderProgram::readShaderSPIRVBytecode(const std::string& path) noexcept {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        return {};
    }

    const std::streamsize size = file.tellg();
    if (size % 4 != 0) {
        file.close();
        return {};
    }

    std::vector<uint32_t> buffer(size / 4);

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(buffer.data()), size);

    file.close();

    return buffer;
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
