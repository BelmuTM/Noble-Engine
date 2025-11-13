#pragma once
#ifndef NOBLEENGINE_FRAMECONTEXT_H
#define NOBLEENGINE_FRAMECONTEXT_H

#include "graphics/vulkan/common/VulkanHeader.h"

struct FrameContext {
    uint32_t          frameIndex = 0;
    vk::CommandBuffer cmdBuffer{};
    vk::ImageView     swapchainImageView{};
    vk::Extent2D      extent{};

    std::vector<vk::DescriptorSet> frameDescriptors;

    FrameContext& setFrameIndex(const uint32_t _frameIndex) { frameIndex = _frameIndex; return *this; }

    FrameContext& setCommandBuffer(const vk::CommandBuffer _cmdBuffer) { cmdBuffer = _cmdBuffer; return *this; }

    FrameContext& setSwapchainImageView(const vk::ImageView _swapchainImageView) {
        swapchainImageView = _swapchainImageView; return *this;
    }

    FrameContext& setExtent(const vk::Extent2D _extent) { extent = _extent; return *this; }
};

#endif // NOBLEENGINE_FRAMECONTEXT_H
