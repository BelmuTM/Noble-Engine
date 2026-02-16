#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "VulkanRenderPassResource.h"

#include <memory>

static constexpr vk::ClearValue defaultClearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

struct VulkanRenderPassAttachment {
    VulkanRenderPassResource resource{};

    vk::AttachmentLoadOp  loadOp  = vk::AttachmentLoadOp::eClear;
    vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;

    vk::ClearValue clearValue = defaultClearColor;

    VulkanRenderPassAttachment& setResource(const VulkanRenderPassResource& _resource) noexcept {
        resource = _resource; return *this;
    }

    VulkanRenderPassAttachment& setLoadOp(const vk::AttachmentLoadOp _loadOp) noexcept {
        loadOp = _loadOp; return *this;
    }

    VulkanRenderPassAttachment& setStoreOp(const vk::AttachmentStoreOp _storeOp) noexcept {
        storeOp = _storeOp; return *this;
    }

    VulkanRenderPassAttachment& setClearValue(const vk::ClearValue _clearValue) noexcept {
        clearValue = _clearValue;
        return *this;
    }

    explicit operator bool() const noexcept {
        return static_cast<bool>(resource.resolveImage());
    }
};

using AttachmentsVector = std::vector<std::unique_ptr<VulkanRenderPassAttachment>>;
