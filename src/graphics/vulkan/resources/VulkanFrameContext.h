#pragma once
#ifndef NOBLEENGINE_VULKANFRAMECONTEXT_H
#define NOBLEENGINE_VULKANFRAMECONTEXT_H

#include "graphics/vulkan/common/VulkanHeader.h"

struct VulkanFrameContext {
    uint32_t frameIndex = 0;

    vk::ImageView swapchainImageView{};
    vk::Extent2D  extent{};

    VulkanFrameContext& setFrameIndex(const uint32_t _frameIndex) noexcept {
        frameIndex = _frameIndex;
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

#endif // NOBLEENGINE_VULKANFRAMECONTEXT_H
