#include "VulkanShaderProgram.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "core/ResourceManager.h"
#include "core/debug/ErrorHandling.h"

#include <fstream>
#include <ranges>
#include <unordered_map>

#include <spirv-reflect/spirv_reflect.h>

#include "core/debug/Logger.h"

static const std::unordered_map<std::string, std::pair<vk::ShaderStageFlagBits, const char*>> stageData = {
    {"vert", {vk::ShaderStageFlagBits::eVertex,   "vertMain"}},
    {"frag", {vk::ShaderStageFlagBits::eFragment, "fragMain"}},
    {"comp", {vk::ShaderStageFlagBits::eCompute,  "compMain"}}
};

void VulkanShaderProgram::destroy() noexcept {
    clearShaderModules();

    _device = VK_NULL_HANDLE;
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
    const std::vector<std::string>& paths, const bool fullscreen, const vk::Device& device, std::string& errorMessage
) {
    if (paths.empty()) {
        errorMessage = "Failed to load shader program: no paths provided";
        return false;
    }

    _device       = device;
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

        const auto [stage, entryPoint] = it->second;

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
            .setStage(stage)
            .setModule(module)
            .setPName(entryPoint);

        _shaderStages.push_back(stageInfo);
        _stageFlags |= stage;

        Logger::debug("----- STAGE [" + path + "] -----");
        TRY(reflectShaderResources(bytecode, stage, errorMessage));
    }

    guard.release();

    return true;
}

bool VulkanShaderProgram::load(
    const std::string& path, const bool fullscreen, const vk::Device& device, std::string& errorMessage
) {
    return loadFromFiles(findShaderFilePaths(path), fullscreen, device, errorMessage);
}

bool VulkanShaderProgram::reflectShaderResources(
    const std::vector<uint32_t>& bytecode, const vk::ShaderStageFlags stage, std::string& errorMessage
) {
    const void*  bytecodeData = bytecode.data();
    const size_t bytecodeSize = bytecode.size() * sizeof(uint32_t);

    SpvReflectShaderModule module;
    SpvReflectResult result = spvReflectCreateShaderModule(bytecodeSize, bytecodeData, &module);

    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        errorMessage = "Failed to reflect SPIR-V shaders: invalid shader bytecode";
        return false;
    }

    // Stage outputs
    Logger::debug("--> Stage Outputs <--");

    uint32_t outputCount = 0;
    result = spvReflectEnumerateOutputVariables(&module, &outputCount, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    std::vector<SpvReflectInterfaceVariable*> outputs(outputCount);
    result = spvReflectEnumerateOutputVariables(&module, &outputCount, outputs.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    for (const auto& output : outputs) {
        if (!output->name) continue;

        if (stage & vk::ShaderStageFlagBits::eFragment) {
            if (output->type_description->type_flags == (SPV_REFLECT_TYPE_FLAG_VECTOR | SPV_REFLECT_TYPE_FLAG_FLOAT)) {
                Logger::debug("location=" + std::to_string(output->location) + " : " + output->name);
                _stageOutputs.emplace_back(output->name);
            }
        }
    }

    // Descriptors
    Logger::debug("--> Descriptors <--");

    uint32_t descriptorSetCount = 0;
    result = spvReflectEnumerateDescriptorSets(&module, &descriptorSetCount, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    std::vector<SpvReflectDescriptorSet*> descriptorSets(descriptorSetCount);
    result = spvReflectEnumerateDescriptorSets(&module, &descriptorSetCount, descriptorSets.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    for (uint32_t set = 0; set < descriptorSetCount; set++) {
        const SpvReflectDescriptorSet* descriptorSet = descriptorSets[set];

        for (uint32_t binding = 0; binding < descriptorSet->binding_count; binding++) {
            const SpvReflectDescriptorBinding* descriptorBinding = descriptorSet->bindings[binding];

            Logger::debug(
                "set=" + std::to_string(descriptorSet->set) + "(" + std::to_string(set) + "), " + "binding=" +
                std::to_string(descriptorBinding->binding) + " : " + descriptorBinding->name
            );

            VulkanDescriptorBindingInfo info{
                .binding    = binding,
                .type       = vk::DescriptorType::eCombinedImageSampler,
                .stageFlags = stage
            };

            _descriptorSchemes[descriptorSet->set].push_back(info);
        }
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

std::vector<std::string> VulkanShaderProgram::findShaderFilePaths(const std::string& path) {
    std::vector<std::string> paths{};
    for (const auto& stageExtension : stageData | std::views::keys) {
        const std::string relativePath = path + "." + stageExtension + ".spv";
        const std::string fullPath     = shaderFilesPath + relativePath;

        if (FILE* file = fopen(fullPath.c_str(), "r")) {
            fclose(file);
            paths.push_back(relativePath);
        }
    }
    return paths;
}
