#pragma once
#ifndef NOBLEENGINE_VULKANRENDERPASSRESOURCE_H
#define NOBLEENGINE_VULKANRENDERPASSRESOURCE_H

#include "graphics/vulkan/resources/images/VulkanImage.h"

#include <functional>

enum VulkanRenderPassResourceType { Texture, Buffer, SwapchainOutput };

struct VulkanRenderPassResource {
    std::string name = "Undefined_Resource";

    VulkanRenderPassResourceType type = Texture;

    VulkanImage* image = nullptr;

    std::function<VulkanImage*()> imageResolver;

    [[nodiscard]] VulkanImage* resolveImage() const {
        if (image) return image;
        return imageResolver();
    }

    VulkanRenderPassResource& setName(const std::string& _name) noexcept { name = _name; return *this; }

    VulkanRenderPassResource& setType(const VulkanRenderPassResourceType _type) noexcept { type = _type; return *this; }

    VulkanRenderPassResource& setImage(VulkanImage* _image) noexcept { image = _image; return *this; }

    VulkanRenderPassResource& setImageResolver(const std::function<VulkanImage*()>& _imageResolver) noexcept {
        imageResolver = _imageResolver;
        return *this;
    }
};

#endif // NOBLEENGINE_VULKANRENDERPASSRESOURCE_H
