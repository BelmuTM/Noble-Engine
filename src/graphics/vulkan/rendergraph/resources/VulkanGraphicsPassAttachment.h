#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/rendergraph/resources/VulkanPassResource.h"

static constexpr vk::ClearValue defaultClearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

struct VulkanGraphicsPassAttachmentDescriptor {
    std::string name{};

    vk::AttachmentLoadOp  loadOp  = vk::AttachmentLoadOp::eClear;
    vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;

    vk::ClearValue clearValue = defaultClearColor;

    VulkanGraphicsPassAttachmentDescriptor& setName(const std::string& _name) noexcept {
        name = _name; return *this;
    }

    VulkanGraphicsPassAttachmentDescriptor& setLoadOp(const vk::AttachmentLoadOp _loadOp) noexcept {
        loadOp = _loadOp; return *this;
    }

    VulkanGraphicsPassAttachmentDescriptor& setStoreOp(const vk::AttachmentStoreOp _storeOp) noexcept {
        storeOp = _storeOp; return *this;
    }

    VulkanGraphicsPassAttachmentDescriptor& setClearValue(const vk::ClearValue _clearValue) noexcept {
        clearValue = _clearValue;
        return *this;
    }
};

struct VulkanGraphicsPassAttachment {
    VulkanGraphicsPassAttachmentDescriptor descriptor{};

    const VulkanPassResource* resource = nullptr;

    explicit VulkanGraphicsPassAttachment(VulkanGraphicsPassAttachmentDescriptor _descriptor)
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

    VulkanGraphicsPassAttachment& setResource(const VulkanPassResource* _resource) noexcept {
        resource = _resource; return *this;
    }
};
