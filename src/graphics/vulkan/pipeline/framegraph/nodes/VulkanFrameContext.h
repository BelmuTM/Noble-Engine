#pragma once
#ifndef NOBLEENGINE_FRAMECONTEXT_H
#define NOBLEENGINE_FRAMECONTEXT_H

#include "graphics/vulkan/common/VulkanHeader.h"

struct VulkanFrameContext {
    uint32_t          frameIndex = 0;
    vk::CommandBuffer cmdBuffer{};
    vk::ImageView     swapchainImageView{};
    vk::Extent2D      extent{};

    std::vector<vk::DescriptorSet> frameDescriptors;

    VulkanFrameContext& setFrameIndex(const uint32_t _frameIndex) noexcept {
        frameIndex = _frameIndex;
        return *this;
    }

    VulkanFrameContext& setCommandBuffer(const vk::CommandBuffer _cmdBuffer) noexcept {
        cmdBuffer = _cmdBuffer;
        return *this;
    }

    VulkanFrameContext& setSwapchainImageView(const vk::ImageView _swapchainImageView) noexcept {
        swapchainImageView = _swapchainImageView;
        return *this;
    }

    VulkanFrameContext& setExtent(const vk::Extent2D _extent) noexcept {
        extent = _extent;
        return *this;
    }
};

#endif // NOBLEENGINE_FRAMECONTEXT_H
