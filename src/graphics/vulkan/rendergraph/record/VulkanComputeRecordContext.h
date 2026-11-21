#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/rendergraph/VulkanFrameContext.h"
#include "graphics/vulkan/rendergraph/nodes/VulkanPassBindings.h"

#include "graphics/vulkan/rendergraph/dispatch/VulkanDispatchCall.h"

#include "VulkanRecordContext.h"

class VulkanComputeRecordContext final : public VulkanRecordContext {
public:
    VulkanComputeRecordContext(
        const vk::CommandBuffer   commandBuffer,
        const VulkanFrameContext& frame,
        const VulkanPassBindings& pass
    ) noexcept
        :
        VulkanRecordContext(commandBuffer, frame, pass)
    {}

    VulkanComputeRecordContext(const VulkanComputeRecordContext&)            = delete;
    VulkanComputeRecordContext& operator=(const VulkanComputeRecordContext&) = delete;

    VulkanComputeRecordContext(VulkanComputeRecordContext&&)            = delete;
    VulkanComputeRecordContext& operator=(VulkanComputeRecordContext&&) = delete;

private:

};
