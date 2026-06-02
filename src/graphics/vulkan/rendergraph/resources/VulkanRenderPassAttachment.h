#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/rendergraph/resources/VulkanRenderPassResource.h"

static constexpr vk::ClearValue defaultClearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

struct VulkanRenderPassAttachmentDescriptor {
    std::string name{};

    vk::AttachmentLoadOp  loadOp  = vk::AttachmentLoadOp::eClear;
    vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;

    vk::ClearValue clearValue = defaultClearColor;

    VulkanRenderPassAttachmentDescriptor& setName(const std::string& _name) noexcept {
        name = _name; return *this;
    }

    VulkanRenderPassAttachmentDescriptor& setLoadOp(const vk::AttachmentLoadOp _loadOp) noexcept {
        loadOp = _loadOp; return *this;
    }

    VulkanRenderPassAttachmentDescriptor& setStoreOp(const vk::AttachmentStoreOp _storeOp) noexcept {
        storeOp = _storeOp; return *this;
    }

    VulkanRenderPassAttachmentDescriptor& setClearValue(const vk::ClearValue _clearValue) noexcept {
        clearValue = _clearValue;
        return *this;
    }
};

struct VulkanRenderPassAttachment {
    VulkanRenderPassAttachmentDescriptor descriptor{};

    const VulkanRenderPassResource* resource = nullptr;

    explicit VulkanRenderPassAttachment(VulkanRenderPassAttachmentDescriptor _descriptor)
        : descriptor(std::move(_descriptor)) {}

    explicit operator bool() const noexcept {
        return static_cast<bool>(resource);
    }

    [[nodiscard]] vk::RenderingAttachmentInfo getInfo() const noexcept {
        const VulkanImage* resourceImage = resource->resolveImage();

        return vk::RenderingAttachmentInfo{}
            .setImageView(resourceImage->getImageView())
            .setImageLayout(resourceImage->getLayout())
            .setLoadOp(descriptor.loadOp)
            .setStoreOp(descriptor.storeOp)
            .setClearValue(descriptor.clearValue);
    }

    VulkanRenderPassAttachment& setResource(const VulkanRenderPassResource* _resource) noexcept {
        resource = _resource; return *this;
    }
};
