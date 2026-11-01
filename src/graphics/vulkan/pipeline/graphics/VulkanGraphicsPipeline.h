#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/pipeline/VulkanPipeline.h"
#include "graphics/vulkan/pipeline/graphics/VulkanGraphicsPipelineState.h"

struct VulkanGraphicsPipelineDescriptor {
    VulkanPipelineLayoutDescriptor layout{};

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages{};

    VulkanGraphicsPipelineState state{};

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

    [[nodiscard]] vk::PipelineBindPoint getBindPoint() const noexcept override {
        return vk::PipelineBindPoint::eGraphics;
    }

private:
    [[nodiscard]] Expected<void> createGraphicsPipeline(
        const vk::Device& device, const VulkanGraphicsPipelineDescriptor& descriptor
    );
};
