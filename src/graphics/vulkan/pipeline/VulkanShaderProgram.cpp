#include "VulkanShaderProgram.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include <filesystem>
#include <fstream>
#include <ranges>
#include <unordered_map>

#include <spirv-reflect/spirv_reflect.h>

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
    if (!_device) return;

    for (const auto& module : _shaderModules) {
        _device.destroyShaderModule(module);
    }

    _shaderModules.clear();
}

Expected<vk::ShaderModule> VulkanShaderProgram::createShaderModule(const std::vector<std::uint32_t>& bytecode) const {
    vk::ShaderModuleCreateInfo shaderModuleInfo{};
    shaderModuleInfo
        .setCodeSize(sizeof(std::uint32_t) * bytecode.size())
        .setPCode(bytecode.data());

    vk::ShaderModule shaderModule;
    VK_CREATE(shaderModule, _device.createShaderModule(shaderModuleInfo));

    return Expected(std::move(shaderModule));
}

Expected<void> VulkanShaderProgram::loadFromFiles(const std::vector<std::string>& paths, const vk::Device& device) {
    if (paths.empty()) {
        return VK_FAIL("Failed to load shader program: no paths found.");
    }

    _device = device;

    ScopeGuard guard{[this] { clearShaderModules(); }};

    for (const auto& path : paths) {
        const std::string baseErrorMessage = "Failed to load shader stage \"" + path + "\": ";

        std::string stageExtension = extractStageExtension(path);

        const auto it = stageData.find(stageExtension);

        if (stageExtension.empty() || it == stageData.end()) {
            return VK_FAIL(baseErrorMessage + "incorrect file extension \"" + stageExtension + "\".");
        }

        const auto [stage, entryPoint] = it->second;

        const std::vector<std::uint32_t>& bytecode = readShaderSPIRVBytecode(path);
        if (bytecode.empty()) {
            return VK_FAIL(baseErrorMessage + "bytecode is empty (file does not exist or is zero bytes).");
        }

        vk::ShaderModule module;
        TRY_ASSIGN(module, createShaderModule(bytecode));

        _shaderModules.push_back(module);

        vk::PipelineShaderStageCreateInfo stageInfo{};
        stageInfo
            .setStage(stage)
            .setModule(module)
            .setPName(entryPoint);

        _shaderStages.push_back(stageInfo);

        //Logger::debug("----- STAGE [" + path + "] -----");
        TRY(reflectShaderResources(bytecode, stage));
    }

    guard.release();

    return {};
}

Expected<void> VulkanShaderProgram::load(const std::string& path, const vk::Device& device) {
    return loadFromFiles(findShaderFilePaths(path), device);
}

Expected<void> VulkanShaderProgram::reflectShaderResources(
    const std::vector<std::uint32_t>& bytecode, const vk::ShaderStageFlags stage
) {
    const void*  bytecodeData = bytecode.data();
    const std::size_t bytecodeSize = bytecode.size() * sizeof(std::uint32_t);

    SpvReflectShaderModule module;
    SpvReflectResult result = spvReflectCreateShaderModule(bytecodeSize, bytecodeData, &module);

    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        return VK_FAIL("Failed to reflect SPIR-V shaders: invalid shader bytecode.");
    }

    constexpr SpvReflectTypeFlags floatVectorFlags = SPV_REFLECT_TYPE_FLAG_VECTOR | SPV_REFLECT_TYPE_FLAG_FLOAT;

    // Stage outputs
    //Logger::debug("--> Stage Outputs <--");

    std::uint32_t outputCount = 0;
    result = spvReflectEnumerateOutputVariables(&module, &outputCount, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    std::vector<SpvReflectInterfaceVariable*> outputs(outputCount);
    result = spvReflectEnumerateOutputVariables(&module, &outputCount, outputs.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    for (const auto& output : outputs) {
        if (!output->name) continue;

        std::string cleanName = output->name;

        if (cleanName.rfind("entryPointParam_", 0) == 0) {
            const std::size_t pos = cleanName.rfind('.');
            if (pos != std::string::npos) {
                cleanName = cleanName.substr(pos + 1);
            }
        }

        if (stage & vk::ShaderStageFlagBits::eFragment) {
            if (output->type_description->type_flags == floatVectorFlags) {
                //Logger::debug("location=" + std::to_string(output->location) + " : " + cleanName);
                _stageOutputs.emplace_back(cleanName);
            }
        }
    }

    // Descriptors
    //Logger::debug("--> Descriptors <--");

    std::uint32_t descriptorSetCount = 0;
    result = spvReflectEnumerateDescriptorSets(&module, &descriptorSetCount, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    std::vector<SpvReflectDescriptorSet*> descriptorSets(descriptorSetCount);
    result = spvReflectEnumerateDescriptorSets(&module, &descriptorSetCount, descriptorSets.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    for (std::uint32_t set = 0; set < descriptorSetCount; set++) {
        const SpvReflectDescriptorSet* descriptorSet = descriptorSets[set];

        for (std::uint32_t binding = 0; binding < descriptorSet->binding_count; binding++) {
            const SpvReflectDescriptorBinding* descriptorBinding = descriptorSet->bindings[binding];

            /*
            Logger::debug(
                "set=" + std::to_string(descriptorSet->set) + "(" + std::to_string(set) + "), " + "binding=" +
                std::to_string(descriptorBinding->binding) + " : " + descriptorBinding->name
            );
            */

            if (descriptorBinding->type_description->type_flags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLED_IMAGE) {

                const VulkanDescriptorBindingInfo info{
                    .binding    = descriptorBinding->binding,
                    .type       = vk::DescriptorType::eCombinedImageSampler,
                    .stageFlags = stage,
                    .name       = descriptorBinding->name
                };

                _descriptorSchemes[descriptorSet->set].push_back(info);
            }
        }
    }

    // Push constants
    //Logger::debug("--> Push Constants <--");

    std::uint32_t pushConstantCount = 0;
    result = spvReflectEnumeratePushConstantBlocks(&module, &pushConstantCount, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    std::vector<SpvReflectBlockVariable*> pushConstants(pushConstantCount);
    result = spvReflectEnumeratePushConstantBlocks(&module, &pushConstantCount, pushConstants.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    for (const auto& pushConstant : pushConstants) {
        const std::string& name = pushConstant->name;

        const VulkanPushConstantRange pushConstantRange{
            .stageFlags = stage,
            .offset     = 0,
            .size       = pushConstant->size
        };

        _pushConstants[name] = pushConstantRange;
    }

    spvReflectDestroyShaderModule(&module);

    return {};
}

std::string VulkanShaderProgram::extractStageExtension(const std::string& path) noexcept {
    const auto lastDot       = path.rfind('.');
    const auto secondLastDot = path.rfind('.', lastDot - 1);
    if (lastDot == std::string::npos || secondLastDot == std::string::npos) {
        return "";
    }
    return path.substr(secondLastDot + 1, lastDot - secondLastDot - 1);
}

std::vector<std::uint32_t> VulkanShaderProgram::readShaderSPIRVBytecode(const std::string& path) noexcept {
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        return {};
    }

    const std::streamsize size = file.tellg();

    if (size % 4 != 0) {
        file.close();
        return {};
    }

    std::vector<std::uint32_t> buffer(size / 4);

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(buffer.data()), size);

    file.close();

    return buffer;
}

std::vector<std::string> VulkanShaderProgram::findShaderFilePaths(const std::string& path) {
    std::vector<std::string> paths{};
    for (const auto& stageExtension : stageData | std::views::keys) {
        const std::string relativePath = path + "." + stageExtension + ".spv";

        if (std::filesystem::exists(relativePath)) {
            paths.push_back(relativePath);
        }
    }
    return paths;
}
