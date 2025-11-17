#pragma once
#ifndef NOBLEENGINE_VULKANSHADERPROGRAM_H
#define NOBLEENGINE_VULKANSHADERPROGRAM_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorManager.h"

#include <unordered_map>
#include <vector>

struct VulkanPushConstantRange {
    vk::ShaderStageFlags stageFlags;
    uint32_t offset = 0;
    uint32_t size   = 0;
};

class VulkanShaderProgram {
public:
    VulkanShaderProgram()  = default;
    ~VulkanShaderProgram() = default;

    VulkanShaderProgram(const VulkanShaderProgram&)            = default;
    VulkanShaderProgram& operator=(const VulkanShaderProgram&) = default;

    VulkanShaderProgram(VulkanShaderProgram&&)            = default;
    VulkanShaderProgram& operator=(VulkanShaderProgram&&) = default;

    void destroy() noexcept;

    [[nodiscard]] bool loadFromFiles(
        const std::vector<std::string>& paths, bool fullscreen, const vk::Device& device, std::string& errorMessage
    );

    [[nodiscard]] bool load(
        const std::string& path, bool fullscreen, const vk::Device& device, std::string& errorMessage
    );

    [[nodiscard]] bool isFullscreen() const noexcept { return _isFullscreen; }

    [[nodiscard]] const std::vector<vk::PipelineShaderStageCreateInfo>& getStages() const noexcept {
        return _shaderStages;
    }

    [[nodiscard]] vk::ShaderStageFlags getStageFlags() const noexcept { return _stageFlags; }

    [[nodiscard]] std::vector<std::string> getStageOutputs() const noexcept { return _stageOutputs; }

    [[nodiscard]] std::unordered_map<uint32_t, VulkanDescriptorScheme> getDescriptorSchemes() const noexcept {
        return _descriptorSchemes;
    }

    [[nodiscard]] const std::unordered_map<std::string, VulkanPushConstantRange>& getPushConstants() const noexcept {
        return _pushConstants;
    }

private:
    vk::Device _device{};

    bool _isFullscreen = false;

    std::vector<vk::ShaderModule>                  _shaderModules{};
    std::vector<vk::PipelineShaderStageCreateInfo> _shaderStages{};
    vk::ShaderStageFlags                           _stageFlags{};

    std::vector<std::string>                                 _stageOutputs{};
    std::unordered_map<uint32_t, VulkanDescriptorScheme>     _descriptorSchemes{};
    std::unordered_map<std::string, VulkanPushConstantRange> _pushConstants{};

    void clearShaderModules();

    vk::ShaderModule createShaderModule(const std::vector<uint32_t>& bytecode, std::string& errorMessage) const;

    [[nodiscard]] bool reflectShaderResources(
        const std::vector<uint32_t>& bytecode, vk::ShaderStageFlags stage, std::string& errorMessage
    );

    static std::string extractStageExtension(const std::string& path) noexcept;

    static std::vector<uint32_t> readShaderSPIRVBytecode(const std::string& path) noexcept;

    static std::vector<std::string> findShaderFilePaths(const std::string& path);
};

#endif //NOBLEENGINE_VULKANSHADERPROGRAM_H
