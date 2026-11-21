#pragma once

#include "graphics/vulkan/rendergraph/nodes/VulkanPass.h"

class VulkanComputeRecordContext;

struct VulkanComputePassDescriptor {
    VulkanPassDescriptor base;

    std::function<Expected<void>(VulkanComputeRecordContext&)> record;
};

class VulkanComputePipeline;

class VulkanComputePass {
public:

    explicit VulkanComputePass(const VulkanComputePassDescriptor& descriptor)
        : _pass(descriptor.base), _computePassDescriptor(descriptor) {}

    ~VulkanComputePass() = default;

    VulkanComputePass(const VulkanComputePass&)            = delete;
    VulkanComputePass& operator=(const VulkanComputePass&) = delete;

    VulkanComputePass(VulkanComputePass&&)            = delete;
    VulkanComputePass& operator=(VulkanComputePass&&) = delete;

    // Getters

    [[nodiscard]]       VulkanPass& base()       noexcept { return _pass; }
    [[nodiscard]] const VulkanPass& base() const noexcept { return _pass; }

    [[nodiscard]] const VulkanComputePassDescriptor& getComputePassDescriptor() const noexcept {
        return _computePassDescriptor;
    }

    [[nodiscard]] const VulkanComputePipeline* getComputePipeline() const noexcept { return _computePipeline; }

    // Setters

    VulkanComputePass& setComputePipeline(const VulkanComputePipeline* computePipeline) noexcept {
        _computePipeline = computePipeline;
        return *this;
    }

private:
    VulkanPass _pass;

    VulkanComputePassDescriptor _computePassDescriptor;

    const VulkanComputePipeline* _computePipeline = nullptr;
};
