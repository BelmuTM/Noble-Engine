#pragma once

#include "graphics/vulkan/resources/images/VulkanImage.h"

#include <functional>
#include <utility>

enum class VulkanRenderPassResourceType : std::uint8_t { Transient, External, SwapchainOutput };

struct VulkanRenderPassResourceDescriptor {
    std::string name = "Undefined_Resource";

    vk::Format format = vk::Format::eR8G8B8A8Unorm;

    VulkanRenderPassResourceType type = VulkanRenderPassResourceType::Transient;

    VulkanRenderPassResourceDescriptor& setName(const std::string& _name) noexcept {
        name = _name; return *this;
    }

    VulkanRenderPassResourceDescriptor& setFormat(const vk::Format _format) noexcept {
        format = _format; return *this;
    }

    VulkanRenderPassResourceDescriptor& setType(const VulkanRenderPassResourceType _type) noexcept {
        type = _type; return *this;
    }
};

struct VulkanRenderPassResource {
    VulkanRenderPassResourceDescriptor descriptor{};

    VulkanImage* image = nullptr;

    std::function<VulkanImage*()> imageResolver;

    explicit VulkanRenderPassResource(VulkanRenderPassResourceDescriptor _descriptor)
        : descriptor(std::move(_descriptor)) {}

    explicit operator bool() const noexcept {
        return static_cast<bool>(resolveImage());
    }

    [[nodiscard]] VulkanImage* resolveImage() const {
        if (image || !imageResolver) return image;
        return imageResolver();
    }

    VulkanRenderPassResource& setImage(VulkanImage* _image) noexcept { image = _image; return *this; }

    VulkanRenderPassResource& setImageResolver(std::function<VulkanImage*()> _imageResolver) noexcept {
        imageResolver = std::move(_imageResolver);
        return *this;
    }
};
