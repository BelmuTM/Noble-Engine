#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "VulkanPipeline.h"

#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"

class VulkanGraphicsPipeline : public VulkanPipeline {
public:
    VulkanGraphicsPipeline()           = default;
    ~VulkanGraphicsPipeline() override = default;

    VulkanGraphicsPipeline(const VulkanGraphicsPipeline&)            = delete;
    VulkanGraphicsPipeline& operator=(const VulkanGraphicsPipeline&) = delete;

    VulkanGraphicsPipeline(VulkanGraphicsPipeline&&)            = delete;
    VulkanGraphicsPipeline& operator=(VulkanGraphicsPipeline&&) = delete;

    [[nodiscard]] Expected<void> create(const vk::Device& device, const VulkanRenderPass& pass) noexcept;

    [[nodiscard]] static vk::PipelineBindPoint getBindPoint() noexcept {
        return vk::PipelineBindPoint::eGraphics;
    }

private:
    [[nodiscard]] Expected<void> createGraphicsPipeline(const vk::Device& device, const VulkanRenderPass& pass);
};
