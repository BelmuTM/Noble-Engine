#pragma once

#include "graphics/vulkan/rendergraph/nodes/VulkanPass.h"

#include "graphics/vulkan/rendergraph/dispatch/VulkanDispatchCall.h"

struct VulkanComputePassDescriptor {
    VulkanPassDescriptor base;
};

class VulkanComputePipeline;

class VulkanComputePass {
public:
    using DispatchCallsVector = std::vector<VulkanDispatchCall>;

    explicit VulkanComputePass(VulkanComputePassDescriptor descriptor)
        : _pass(std::move(descriptor.base)), _computePassDescriptor(std::move(descriptor)) {}

    ~VulkanComputePass() = default;

    VulkanComputePass(const VulkanComputePass&)            = delete;
    VulkanComputePass& operator=(const VulkanComputePass&) = delete;

    VulkanComputePass(VulkanComputePass&&)            = delete;
    VulkanComputePass& operator=(VulkanComputePass&&) = delete;

    void destroy() const noexcept;

    // Getters

    [[nodiscard]]       VulkanPass& base()       noexcept { return _pass; }
    [[nodiscard]] const VulkanPass& base() const noexcept { return _pass; }

    [[nodiscard]] const VulkanComputePassDescriptor& getComputePassDescriptor() const noexcept {
        return _computePassDescriptor;
    }

    [[nodiscard]] const VulkanComputePipeline* getComputePipeline() const noexcept { return _computePipeline; }

    [[nodiscard]]       DispatchCallsVector& getDispatchCalls()       noexcept { return _dispatchCalls; }
    [[nodiscard]] const DispatchCallsVector& getDispatchCalls() const noexcept { return _dispatchCalls; }

    // Setters

    VulkanComputePass& setComputePipeline(const VulkanComputePipeline* computePipeline) noexcept {
        _computePipeline = computePipeline;
        return *this;
    }

    VulkanDispatchCall& emplaceDispatchCall() noexcept {
        return _dispatchCalls.emplace_back();
    }

private:
    VulkanPass _pass;

    VulkanComputePassDescriptor _computePassDescriptor;

    const VulkanComputePipeline*  _computePipeline = nullptr;

    DispatchCallsVector _dispatchCalls{};
};
