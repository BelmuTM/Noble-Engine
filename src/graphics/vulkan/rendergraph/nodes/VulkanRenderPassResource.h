#pragma once

#include "graphics/vulkan/resources/images/VulkanImage.h"

#include <functional>

enum class VulkanRenderPassResourceType { Buffer, SwapchainOutput };

struct VulkanRenderPassResource {
    std::string name = "Undefined_Resource";

    VulkanRenderPassResourceType type = VulkanRenderPassResourceType::Buffer;

    VulkanImage* image = nullptr;

    std::function<VulkanImage*()> imageResolver;

    [[nodiscard]] VulkanImage* resolveImage() const {
        if (image) return image;
        return imageResolver();
    }

    VulkanRenderPassResource& setName(const std::string& _name) noexcept { name = _name; return *this; }

    VulkanRenderPassResource& setType(const VulkanRenderPassResourceType _type) noexcept { type = _type; return *this; }

    VulkanRenderPassResource& setImage(VulkanImage* _image) noexcept { image = _image; return *this; }

    VulkanRenderPassResource& setImageResolver(std::function<VulkanImage*()> _imageResolver) noexcept {
        imageResolver = std::move(_imageResolver);
        return *this;
    }
};
