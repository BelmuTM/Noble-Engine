#pragma once

#include "core/debug/ErrorHandling.h"

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

    [[nodiscard]] Expected<void> create(const vk::Device& device) noexcept;

    void destroy() noexcept;

    [[nodiscard]] VulkanGraphicsPipeline* allocatePipeline();

    [[nodiscard]] Expected<void> createGraphicsPipeline(
        VulkanGraphicsPipeline* graphicsPipeline, const VulkanRenderPass& pass
    ) const;

private:
    vk::Device _device{};

    std::vector<std::unique_ptr<VulkanGraphicsPipeline>> _graphicsPipelines{};
};
