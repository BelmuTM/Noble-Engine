#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/pipeline/VulkanPipeline.h"

#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"

struct VulkanComputePipelineDescriptor {
    VulkanPipelineLayoutDescriptor                 layout{};
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages{};
};

class VulkanComputePipeline final : public VulkanPipeline {
public:
    VulkanComputePipeline()           = default;
    ~VulkanComputePipeline() override = default;

    VulkanComputePipeline(const VulkanComputePipeline&)            = delete;
    VulkanComputePipeline& operator=(const VulkanComputePipeline&) = delete;

    VulkanComputePipeline(VulkanComputePipeline&&)            = delete;
    VulkanComputePipeline& operator=(VulkanComputePipeline&&) = delete;

    [[nodiscard]] Expected<void> create(
        const vk::Device& device, const VulkanComputePipelineDescriptor& descriptor
    ) noexcept;

    [[nodiscard]] static vk::PipelineBindPoint getBindPoint() noexcept {
        return vk::PipelineBindPoint::eCompute;
    }

private:
    [[nodiscard]] Expected<void> createComputePipeline(
        const vk::Device& device, const VulkanComputePipelineDescriptor& descriptor
    );
};
