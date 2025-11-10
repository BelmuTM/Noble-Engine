#pragma once
#ifndef NOBLEENGINE_VULKANPIPELINEMANAGER_H
#define NOBLEENGINE_VULKANPIPELINEMANAGER_H

#include "graphics/vulkan/common/VulkanHeader.h"
#include "graphics/vulkan/core/VulkanSwapchain.h"
#include "VulkanGraphicsPipeline.h"

class VulkanPipelineManager {
public:
    VulkanPipelineManager()  = default;
    ~VulkanPipelineManager() = default;

    VulkanPipelineManager(const VulkanPipelineManager&)            = delete;
    VulkanPipelineManager& operator=(const VulkanPipelineManager&) = delete;

    VulkanPipelineManager(VulkanPipelineManager&&)            = delete;
    VulkanPipelineManager& operator=(VulkanPipelineManager&&) = delete;

    [[nodiscard]] bool create(
        const vk::Device& device, const VulkanSwapchain& swapchain, std::string& errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] bool createGraphicsPipeline(
        VulkanGraphicsPipeline&                     graphicsPipeline,
        const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
        const VulkanShaderProgram&                  shaderProgram,
        std::string&                                errorMessage
    );

    [[nodiscard]] bool createGraphicsPipeline(
        VulkanGraphicsPipeline&                     graphicsPipeline,
        const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
        uint32_t                                    pushConstantRangeSize,
        const VulkanShaderProgram&                  shaderProgram,
        std::string&                                errorMessage
    );

private:
    vk::Device _device{};

    const VulkanSwapchain* _swapchain = nullptr;

    std::vector<VulkanGraphicsPipeline*> _graphicsPipelines{};
};

#endif // NOBLEENGINE_VULKANPIPELINEMANAGER_H
