#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/pipeline/VulkanPipeline.h"

struct VulkanComputePipelineDefinition {
    std::string name = "Undefined_Compute_Pipeline";

    std::string programPath;

    bool operator==(const VulkanComputePipelineDefinition&) const = default;
};

struct VulkanComputePipelineDescriptor {
    vk::PipelineShaderStageCreateInfo shaderStage;
    VulkanPipelineLayoutDescriptor    layout{};
};

class VulkanComputePipeline final : public VulkanPipeline {
public:
    VulkanComputePipeline() = default;

    VulkanComputePipeline(const VulkanComputePipeline&)            = delete;
    VulkanComputePipeline& operator=(const VulkanComputePipeline&) = delete;

    VulkanComputePipeline(VulkanComputePipeline&&)            = delete;
    VulkanComputePipeline& operator=(VulkanComputePipeline&&) = delete;

    [[nodiscard]] Expected<void> create(
        const vk::Device& device, const VulkanComputePipelineDescriptor& descriptor
    ) noexcept;

    [[nodiscard]] vk::PipelineBindPoint getBindPoint() const noexcept override {
        return vk::PipelineBindPoint::eCompute;
    }

private:
    [[nodiscard]] Expected<void> createComputePipeline(
        const vk::Device& device, const VulkanComputePipelineDescriptor& descriptor
    );
};
