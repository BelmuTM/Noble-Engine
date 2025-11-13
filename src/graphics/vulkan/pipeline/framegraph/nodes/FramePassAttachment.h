#pragma once
#ifndef NOBLEENGINE_FRAMEPASSATTACHMENT_H
#define NOBLEENGINE_FRAMEPASSATTACHMENT_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/pipeline/framegraph/nodes/FrameResource.h"

static constexpr vk::ClearValue defaultClearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

struct FramePassAttachment {
    FrameResource resource{};

    vk::AttachmentLoadOp  loadOp  = vk::AttachmentLoadOp::eClear;
    vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;

    vk::ClearValue clearValue = defaultClearColor;

    FramePassAttachment& setResource(const FrameResource& _resource) { resource = _resource; return *this; }

    FramePassAttachment& setLoadOp(const vk::AttachmentLoadOp _loadOp) { loadOp = _loadOp; return *this; }

    FramePassAttachment& setStoreOp(const vk::AttachmentStoreOp _storeOp) { storeOp = _storeOp; return *this; }

    FramePassAttachment& setClearValue(const vk::ClearValue _clearValue) { clearValue = _clearValue; return *this; }

    explicit operator bool() const noexcept {
        return static_cast<bool>(resource.resolveImageView);
    }
};

#endif // NOBLEENGINE_FRAMEPASSATTACHMENT_H
