#pragma once
#ifndef NOBLEENGINE_FRAMERESOURCE_H
#define NOBLEENGINE_FRAMERESOURCE_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/pipeline/framegraph/nodes/FrameContext.h"

#include <functional>

enum FrameResourceType { Texture, Buffer, SwapchainOutput };

struct FrameResource {
    std::string       path = "Undefined_Resource";
    FrameResourceType type = Texture;
    vk::Image         image{};
    vk::ImageView     imageView{};
    vk::ImageLayout   layout = vk::ImageLayout::eUndefined;

    std::function<vk::ImageView(const FrameContext&)> resolveImageView;

    FrameResource& setPath(const std::string& _path) { path = _path; return *this; }

    FrameResource& setType(const FrameResourceType _type) { type = _type; return *this; }

    FrameResource& setImage(const vk::Image _image) { image = _image; return *this; }

    FrameResource& setImageView(const vk::ImageView _imageView) { imageView = _imageView; return *this; }

    FrameResource& setLayout(const vk::ImageLayout _layout) { layout = _layout; return *this; }

    FrameResource& setResolveImageView(const std::function<vk::ImageView(const FrameContext&)>& _resolveImageView) {
        resolveImageView = _resolveImageView;
        return *this;
    }
};

#endif // NOBLEENGINE_FRAMERESOURCE_H
