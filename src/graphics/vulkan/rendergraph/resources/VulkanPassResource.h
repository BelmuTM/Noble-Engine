#pragma once

#include "graphics/vulkan/resources/images/VulkanImage.h"

#include <functional>
#include <utility>

enum class VulkanPassResourceType : std::uint8_t { Transient, External, SwapchainOutput };

struct VulkanPassResourceDescriptor {
    std::string name = "Undefined_Resource";

    vk::Format format = vk::Format::eR8G8B8A8Unorm;

    VulkanPassResourceType type = VulkanPassResourceType::Transient;

    VulkanPassResourceDescriptor& setName(const std::string& _name) noexcept {
        name = _name; return *this;
    }

    VulkanPassResourceDescriptor& setFormat(const vk::Format _format) noexcept {
        format = _format; return *this;
    }

    VulkanPassResourceDescriptor& setType(const VulkanPassResourceType _type) noexcept {
        type = _type; return *this;
    }
};

struct VulkanPassResource {
    VulkanPassResourceDescriptor descriptor{};

    VulkanImage* image = nullptr;

    std::function<VulkanImage*()> imageResolver;

    explicit VulkanPassResource(VulkanPassResourceDescriptor _descriptor)
        : descriptor(std::move(_descriptor)) {}

    explicit operator bool() const noexcept {
        return static_cast<bool>(resolveImage());
    }

    [[nodiscard]] VulkanImage* resolveImage() const {
        if (image || !imageResolver) return image;
        return imageResolver();
    }

    VulkanPassResource& setImage(VulkanImage* _image) noexcept { image = _image; return *this; }

    VulkanPassResource& setImageResolver(std::function<VulkanImage*()> _imageResolver) noexcept {
        imageResolver = std::move(_imageResolver);
        return *this;
    }
};
