#pragma once
#ifndef NOBLEENGINE_VULKANPIPELINEMANAGER_H
#define NOBLEENGINE_VULKANPIPELINEMANAGER_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "VulkanGraphicsPipeline.h"

#include <memory>

class VulkanPipelineManager {
public:
    VulkanPipelineManager()  = default;
    ~VulkanPipelineManager() = default;

    VulkanPipelineManager(const VulkanPipelineManager&)            = delete;
    VulkanPipelineManager& operator=(const VulkanPipelineManager&) = delete;

    VulkanPipelineManager(VulkanPipelineManager&&)            = delete;
    VulkanPipelineManager& operator=(VulkanPipelineManager&&) = delete;

    [[nodiscard]] bool create(const vk::Device& device, std::string& errorMessage) noexcept;

    void destroy() noexcept;

    [[nodiscard]] VulkanGraphicsPipeline* allocatePipeline();

    [[nodiscard]] bool createGraphicsPipeline(
        VulkanGraphicsPipeline*         graphicsPipeline,
        const VulkanPipelineDescriptor& descriptor,
        const AttachmentsVector&        colorAttachments,
        std::string&                    errorMessage
    ) const;

private:
    vk::Device _device{};

    std::vector<std::unique_ptr<VulkanGraphicsPipeline>> _graphicsPipelines{};
};

#endif // NOBLEENGINE_VULKANPIPELINEMANAGER_H
