#pragma once
#ifndef NOBLEENGINE_VULKANSHADERPROGRAM_H
#define NOBLEENGINE_VULKANSHADERPROGRAM_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include <vector>

class VulkanShaderProgram {
public:
    explicit VulkanShaderProgram(const vk::Device* device) : _device(device) {
    }

    ~VulkanShaderProgram();

    VulkanShaderProgram(const VulkanShaderProgram&)            = delete;
    VulkanShaderProgram& operator=(const VulkanShaderProgram&) = delete;
    VulkanShaderProgram(VulkanShaderProgram&&)                 = delete;
    VulkanShaderProgram& operator=(VulkanShaderProgram&&)      = delete;

    bool loadFromFiles(const std::vector<std::string>& shaderPaths, std::string& errorMessage);

    [[nodiscard]] const std::vector<vk::PipelineShaderStageCreateInfo>& getStages() const { return shaderStages; }

private:
    const vk::Device* _device;

    std::vector<vk::ShaderModule>                  shaderModules;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    void clearShaderModules();

    static std::string extractStageExtension(const std::string& path) noexcept;
    static std::vector<char> readShaderFile(const std::string& path) noexcept;

    vk::ShaderModule createShaderModule(const std::vector<char>& code, std::string& errorMessage) const;
};

#endif //NOBLEENGINE_VULKANSHADERPROGRAM_H
