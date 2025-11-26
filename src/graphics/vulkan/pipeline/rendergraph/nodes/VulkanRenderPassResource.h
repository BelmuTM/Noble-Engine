#pragma once
#ifndef NOBLEENGINE_VULKANRENDERPASSRESOURCE_H
#define NOBLEENGINE_VULKANRENDERPASSRESOURCE_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/resources/images/VulkanImage.h"

#include <functional>

enum FramePassResourceType { Texture, Buffer, SwapchainOutput };

struct VulkanRenderPassResource {
    std::string name = "Undefined_Resource";

    FramePassResourceType type = Texture;

    const VulkanImage* image = nullptr;

    vk::Image     imageHandle{};
    vk::ImageView imageView{};

    vk::Format format = vk::Format::eUndefined;

    vk::ImageLayout layout = vk::ImageLayout::eUndefined;

    std::function<vk::ImageView()> imageViewResolver = nullptr;

    vk::ImageView resolveImageView() const {
        if (image && image->getImageView() != VK_NULL_HANDLE) {
            return image->getImageView();
        }

        return imageViewResolver();
    }

    VulkanRenderPassResource& setName(const std::string& _name) noexcept { name = _name; return *this; }

    VulkanRenderPassResource& setType(const FramePassResourceType _type) noexcept { type = _type; return *this; }

    VulkanRenderPassResource& setImage(const VulkanImage* _image) noexcept { image = _image; return *this; }

    VulkanRenderPassResource& setImageHandle(const vk::Image _image) noexcept { imageHandle = _image; return *this; }

    VulkanRenderPassResource& setImageView(const vk::ImageView _imageView) noexcept {
        imageView = _imageView;
        return *this;
    }

    VulkanRenderPassResource& setFormat(const vk::Format _format) noexcept { format = _format; return *this; }

    VulkanRenderPassResource& setLayout(const vk::ImageLayout _layout) noexcept { layout = _layout; return *this; }

    VulkanRenderPassResource& setImageViewResolver(const std::function<vk::ImageView()>& _imageViewResolver) noexcept {
        imageViewResolver = _imageViewResolver;
        return *this;
    }
};

#endif // NOBLEENGINE_VULKANRENDERPASSRESOURCE_H
