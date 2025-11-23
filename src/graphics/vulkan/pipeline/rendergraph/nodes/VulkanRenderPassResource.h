#pragma once
#ifndef NOBLEENGINE_VULKANRENDERPASSRESOURCE_H
#define NOBLEENGINE_VULKANRENDERPASSRESOURCE_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/resources/VulkanFrameContext.h"
#include "graphics/vulkan/resources/images/VulkanImage.h"

#include <functional>

enum FramePassResourceType { Texture, Buffer, SwapchainOutput };

struct VulkanRenderPassResource {
    std::string name = "Undefined_Resource";

    FramePassResourceType type = Texture;

    const VulkanImage* imageObject = nullptr;

    vk::Image     imageHandle{};
    vk::ImageView imageView{};

    vk::ImageLayout layout = vk::ImageLayout::eUndefined;
    vk::Format      format = vk::Format::eUndefined;

    vk::ImageLayout currentLayout = vk::ImageLayout::eUndefined;

    std::function<vk::ImageView(const VulkanFrameContext&)> resolveImageView;

    VulkanRenderPassResource& setName(const std::string& _name) noexcept { name = _name; return *this; }

    VulkanRenderPassResource& setType(const FramePassResourceType _type) noexcept { type = _type; return *this; }

    VulkanRenderPassResource& setImageObject(const VulkanImage* _image) noexcept { imageObject = _image; return *this; }

    VulkanRenderPassResource& setImageHandle(const vk::Image _image) noexcept { imageHandle = _image; return *this; }

    VulkanRenderPassResource& setImageView(const vk::ImageView _imageView) noexcept {
        imageView = _imageView;
        return *this;
    }

    VulkanRenderPassResource& setLayout(const vk::ImageLayout _layout) noexcept { layout = _layout; return *this; }

    VulkanRenderPassResource& setFormat(const vk::Format _format) noexcept { format = _format; return *this; }

    VulkanRenderPassResource& setResolveImageView(
        const std::function<vk::ImageView(const VulkanFrameContext&)>& _resolveImageView
    ) noexcept {
        resolveImageView = _resolveImageView;
        return *this;
    }
};

#endif // NOBLEENGINE_VULKANRENDERPASSRESOURCE_H
