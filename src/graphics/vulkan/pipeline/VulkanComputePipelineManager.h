#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "VulkanComputePipeline.h"

#include <memory>
#include <vector>

class VulkanComputePipelineManager {
public:
    VulkanComputePipelineManager()  = default;
    ~VulkanComputePipelineManager() = default;

    VulkanComputePipelineManager(const VulkanComputePipelineManager&)            = delete;
    VulkanComputePipelineManager& operator=(const VulkanComputePipelineManager&) = delete;

    VulkanComputePipelineManager(VulkanComputePipelineManager&&)            = delete;
    VulkanComputePipelineManager& operator=(VulkanComputePipelineManager&&) = delete;

    [[nodiscard]] Expected<void> create(const vk::Device& device) noexcept;

    void destroy() noexcept;

    [[nodiscard]] Expected<VulkanComputePipeline*> createComputePipeline(
        const VulkanComputePipelineDescriptor& descriptor
    );

private:
    vk::Device _device{};

    std::vector<std::unique_ptr<VulkanComputePipeline>> _computePipelines{};
};
