#pragma once
#ifndef NOBLEENGINE_VULKANSHADERPROGRAM_H
#define NOBLEENGINE_VULKANSHADERPROGRAM_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include <vector>

class VulkanShaderProgram {
public:
    explicit VulkanShaderProgram(const vk::Device& device) : _device(device) {}

    ~VulkanShaderProgram();

    VulkanShaderProgram(const VulkanShaderProgram&)            = delete;
    VulkanShaderProgram& operator=(const VulkanShaderProgram&) = delete;

    VulkanShaderProgram(VulkanShaderProgram&&)            = delete;
    VulkanShaderProgram& operator=(VulkanShaderProgram&&) = delete;

    [[nodiscard]] bool loadFromFiles(const std::vector<std::string>& paths, bool fullscreen, std::string& errorMessage);

    [[nodiscard]] bool load(const std::string& name, bool fullscreen, std::string& errorMessage);

    [[nodiscard]] bool isFullscreen() const noexcept { return _isFullscreen; }

    [[nodiscard]] const std::vector<vk::PipelineShaderStageCreateInfo>& getStages() const noexcept {
        return _shaderStages;
    }

    [[nodiscard]] vk::ShaderStageFlags getStageFlags() const noexcept { return _stageFlags; }

    [[nodiscard]] std::vector<vk::DescriptorSetLayout> getDescriptorSetLayouts() const noexcept {
        return _descriptorSetLayouts;
    }

private:
    const vk::Device _device{};

    bool _isFullscreen = false;

    std::vector<vk::ShaderModule>                  _shaderModules{};
    std::vector<vk::PipelineShaderStageCreateInfo> _shaderStages{};
    vk::ShaderStageFlags                           _stageFlags;

    //std::vector<DescriptorBindingInfo>   _bindings{};
    std::vector<vk::DescriptorSetLayout> _descriptorSetLayouts{};

    vk::ShaderModule createShaderModule(const std::vector<uint32_t>& bytecode, std::string& errorMessage) const;

    void clearShaderModules();

    /*
    [[nodiscard]] bool reflectDescriptors(
        const std::vector<uint32_t>& bytecode, vk::ShaderStageFlags stageFlag, std::string& errorMessage
    );
    */

    static std::string extractStageExtension(const std::string& path) noexcept;

    static std::vector<uint32_t> readShaderSPIRVBytecode(const std::string& path) noexcept;

    static std::vector<std::string> findShaderFilePaths(const std::string& name);
};

#endif //NOBLEENGINE_VULKANSHADERPROGRAM_H
