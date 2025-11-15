#include "VulkanShaderProgram.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "core/ResourceManager.h"
#include "core/debug/ErrorHandling.h"

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
    for (const auto& module : _shaderModules) {
        _device.destroyShaderModule(module);
    }
    _shaderModules.clear();

    _device = VK_NULL_HANDLE;
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

        const auto [stageFlags, entryPoint] = it->second;

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
            .setStage(stageFlags)
            .setModule(module)
            .setPName(entryPoint);

        _shaderStages.push_back(stageInfo);
        _stageFlags |= stageFlags;

        //reflectShaderResources(bytecode, stageFlags);
    }

    guard.release();

    return true;
}

bool VulkanShaderProgram::load(
    const std::string& path, const bool fullscreen, const vk::Device& device, std::string& errorMessage
) {
    return loadFromFiles(findShaderFilePaths(path), fullscreen, device, errorMessage);
}

/*
void VulkanShaderProgram::reflectShaderResources(
    const std::vector<uint32_t>& bytecode, const vk::ShaderStageFlags stageFlags
) {
    spirv_cross::Compiler compiler(std::move(bytecode));
    const spirv_cross::ShaderResources resources = compiler.get_shader_resources();

    // Stage outputs
    for (const auto& stageOutput : resources.stage_outputs) {
        const std::string& outputName = compiler.get_name(stageOutput.id);

        const auto& outputType = compiler.get_type(stageOutput.type_id);
        if (outputType.image.format == spv::ImageFormatUnknown) continue;

        _stageOutputs.push_back(outputName);
    }

    // Sampled images (texture2D,...)
    for (const auto& imageSampler : resources.sampled_images) {
        const uint32_t set     = compiler.get_decoration(imageSampler.id, spv::DecorationDescriptorSet);
        const uint32_t binding = compiler.get_decoration(imageSampler.id, spv::DecorationBinding);

        VulkanDescriptorBindingInfo info{
            .binding    = binding,
            .type       = vk::DescriptorType::eCombinedImageSampler,
            .stageFlags = stageFlags
        };

        _descriptorSchemes[set].push_back(info);
    }

    // Storage images (image2D,...)
    for (const auto& storageImage : resources.storage_images) {
        const uint32_t set     = compiler.get_decoration(storageImage.id, spv::DecorationDescriptorSet);
        const uint32_t binding = compiler.get_decoration(storageImage.id, spv::DecorationBinding);

        VulkanDescriptorBindingInfo info{
            .binding    = binding,
            .type       = vk::DescriptorType::eStorageImage,
            .stageFlags = stageFlags
        };

        _descriptorSchemes[set].push_back(info);
    }
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
