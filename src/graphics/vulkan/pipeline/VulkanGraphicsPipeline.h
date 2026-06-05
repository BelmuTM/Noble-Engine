#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "VulkanPipeline.h"

#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"

struct VulkanGraphicsPipelineDescriptor {
    VulkanPipelineLayoutDescriptor layout{};

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages{};

    VulkanRenderPassType passType = VulkanRenderPassType::None;

    std::vector<vk::Format> colorAttachmentFormats{};
    vk::Format              depthAttachmentFormat = vk::Format::eUndefined;
};

class VulkanGraphicsPipeline final : public VulkanPipeline {
public:
    VulkanGraphicsPipeline()           = default;
    ~VulkanGraphicsPipeline() override = default;

    VulkanGraphicsPipeline(const VulkanGraphicsPipeline&)            = delete;
    VulkanGraphicsPipeline& operator=(const VulkanGraphicsPipeline&) = delete;

    VulkanGraphicsPipeline(VulkanGraphicsPipeline&&)            = delete;
    VulkanGraphicsPipeline& operator=(VulkanGraphicsPipeline&&) = delete;

    [[nodiscard]] Expected<void> create(
        const vk::Device& device, const VulkanGraphicsPipelineDescriptor& descriptor
    ) noexcept;

    [[nodiscard]] static vk::PipelineBindPoint getBindPoint() noexcept {
        return vk::PipelineBindPoint::eGraphics;
    }

private:
    [[nodiscard]] Expected<void> createGraphicsPipeline(
        const vk::Device& device, const VulkanGraphicsPipelineDescriptor& descriptor
    );
};
