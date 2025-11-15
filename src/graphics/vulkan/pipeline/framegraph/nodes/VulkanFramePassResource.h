#pragma once
#ifndef NOBLEENGINE_FRAMERESOURCE_H
#define NOBLEENGINE_FRAMERESOURCE_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/pipeline/framegraph/nodes/VulkanFrameContext.h"

#include <functional>

enum FramePassResourceType { Texture, Buffer, SwapchainOutput };

struct VulkanFramePassResource {
    std::string path = "Undefined_Resource";

    FramePassResourceType type = Texture;

    vk::Image       image{};
    vk::ImageView   imageView{};
    vk::ImageLayout layout = vk::ImageLayout::eUndefined;

    std::function<vk::ImageView(const VulkanFrameContext&)> resolveImageView;

    VulkanFramePassResource& setPath(const std::string& _path) noexcept { path = _path; return *this; }

    VulkanFramePassResource& setType(const FramePassResourceType _type) noexcept { type = _type; return *this; }

    VulkanFramePassResource& setImage(const vk::Image _image) noexcept { image = _image; return *this; }

    VulkanFramePassResource& setImageView(const vk::ImageView _imageView) noexcept {
        imageView = _imageView;
        return *this;
    }

    VulkanFramePassResource& setLayout(const vk::ImageLayout _layout) noexcept { layout = _layout; return *this; }

    VulkanFramePassResource& setResolveImageView(
        const std::function<vk::ImageView(const VulkanFrameContext&)>& _resolveImageView
    ) noexcept {
        resolveImageView = _resolveImageView;
        return *this;
    }
};

#endif // NOBLEENGINE_FRAMERESOURCE_H
