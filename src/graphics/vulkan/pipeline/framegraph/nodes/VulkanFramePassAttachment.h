#pragma once
#ifndef NOBLEENGINE_FRAMEPASSATTACHMENT_H
#define NOBLEENGINE_FRAMEPASSATTACHMENT_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/pipeline/framegraph/nodes/VulkanFramePassResource.h"

static constexpr vk::ClearValue defaultClearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

struct VulkanFramePassAttachment {
    VulkanFramePassResource resource{};

    vk::AttachmentLoadOp  loadOp  = vk::AttachmentLoadOp::eClear;
    vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;

    vk::ClearValue clearValue = defaultClearColor;

    VulkanFramePassAttachment& setResource(const VulkanFramePassResource& _resource) noexcept {
        resource = _resource; return *this;
    }

    VulkanFramePassAttachment& setLoadOp(const vk::AttachmentLoadOp _loadOp) noexcept {
        loadOp = _loadOp; return *this;
    }

    VulkanFramePassAttachment& setStoreOp(const vk::AttachmentStoreOp _storeOp) noexcept {
        storeOp = _storeOp; return *this;
    }

    VulkanFramePassAttachment& setClearValue(const vk::ClearValue _clearValue) noexcept {
        clearValue = _clearValue;
        return *this;
    }

    explicit operator bool() const noexcept {
        return static_cast<bool>(resource.resolveImageView);
    }
};

#endif // NOBLEENGINE_FRAMEPASSATTACHMENT_H
