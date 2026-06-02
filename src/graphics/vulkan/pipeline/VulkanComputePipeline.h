#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "VulkanPipeline.h"

#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"

class VulkanComputePipeline : public VulkanPipeline {
public:
    VulkanComputePipeline()           = default;
    ~VulkanComputePipeline() override = default;

    VulkanComputePipeline(const VulkanComputePipeline&)            = delete;
    VulkanComputePipeline& operator=(const VulkanComputePipeline&) = delete;

    VulkanComputePipeline(VulkanComputePipeline&&)            = delete;
    VulkanComputePipeline& operator=(VulkanComputePipeline&&) = delete;

    [[nodiscard]] Expected<void> create(const vk::Device& device, const VulkanRenderPass& pass) noexcept;

    [[nodiscard]] static vk::PipelineBindPoint getBindPoint() noexcept {
        return vk::PipelineBindPoint::eCompute;
    }

private:
    [[nodiscard]] Expected<void> createComputePipeline(const vk::Device& device, const VulkanRenderPass& pass);
};
