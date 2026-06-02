#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "VulkanGraphicsPipeline.h"

#include <memory>
#include <vector>

class VulkanGraphicsPipelineManager {
public:
    VulkanGraphicsPipelineManager()  = default;
    ~VulkanGraphicsPipelineManager() = default;

    VulkanGraphicsPipelineManager(const VulkanGraphicsPipelineManager&)            = delete;
    VulkanGraphicsPipelineManager& operator=(const VulkanGraphicsPipelineManager&) = delete;

    VulkanGraphicsPipelineManager(VulkanGraphicsPipelineManager&&)            = delete;
    VulkanGraphicsPipelineManager& operator=(VulkanGraphicsPipelineManager&&) = delete;

    [[nodiscard]] Expected<void> create(const vk::Device& device) noexcept;

    void destroy() noexcept;

    [[nodiscard]] Expected<VulkanGraphicsPipeline*> createGraphicsPipeline(const VulkanRenderPass& pass);

private:
    vk::Device _device{};

    std::vector<std::unique_ptr<VulkanGraphicsPipeline>> _graphicsPipelines{};
};
