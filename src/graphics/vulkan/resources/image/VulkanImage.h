#pragma once
#ifndef NOBLEENGINE_VULKANIMAGE_H
#define NOBLEENGINE_VULKANIMAGE_H

#include "graphics/vulkan/common/VulkanHeader.h"
#include "graphics/vulkan/core/VulkanCommandManager.h"
#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/resources/descriptors/VulkanDescriptorInfo.h"

#include "graphics/vulkan/core/memory/VmaUsage.h"

class VulkanImage {
public:
    VulkanImage()  = default;
    ~VulkanImage() = default;

    // Implicit conversion operator
    operator vk::Image() const { return _image; }

    VulkanImage(const VulkanImage&)            noexcept = default;
    VulkanImage& operator=(const VulkanImage&) noexcept = default;

    VulkanImage(VulkanImage&&)            noexcept = default;
    VulkanImage& operator=(VulkanImage&&) noexcept = default;

    void destroy(const VulkanDevice& device) noexcept;

    [[nodiscard]] vk::ImageView getImageView() const noexcept { return _imageView; }

    [[nodiscard]] vk::Sampler getSampler() const noexcept { return _sampler; }

    [[nodiscard]] vk::Format getFormat() const noexcept { return _format; }
    void setFormat(const vk::Format format) noexcept { _format = format; }

    [[nodiscard]] vk::Extent3D getExtent() const noexcept { return _extent; }
    void setExtent(const vk::Extent3D extent) noexcept { _extent = extent; }

    [[nodiscard]] bool createImage(
        vk::Extent3D        extent,
        vk::ImageType       type,
        vk::Format          format,
        vk::ImageUsageFlags usage,
        VmaMemoryUsage      memoryUsage,
        const VulkanDevice* device,
        std::string&        errorMessage
    );

    [[nodiscard]] bool createImageView(
        vk::ImageViewType    type,
        vk::Format           format,
        vk::ImageAspectFlags aspectFlags,
        const VulkanDevice*  device,
        std::string&         errorMessage
    );

    [[nodiscard]] bool createSampler(
        vk::Filter             filter,
        vk::SamplerAddressMode addressMode,
        const VulkanDevice*    device,
        std::string&           errorMessage
    );

    [[nodiscard]] DescriptorInfo getDescriptorInfo(const uint32_t binding) const noexcept {
        return {
            .type      = vk::DescriptorType::eCombinedImageSampler,
            .imageInfo = {_sampler, _imageView, vk::ImageLayout::eShaderReadOnlyOptimal},
            .binding   = binding
        };
    }

    [[nodiscard]] bool copyBufferToImage(
        const vk::Buffer&           buffer,
        vk::Extent3D                extent,
        const VulkanCommandManager* commandManager,
        std::string&                errorMessage
    ) const;

    [[nodiscard]] static bool hasStencilComponent(const vk::Format format) {
        return format == vk::Format::eD32SfloatS8Uint
            || format == vk::Format::eD24UnormS8Uint;
    }

    [[nodiscard]] bool transitionImageLayout(
        vk::ImageLayout             oldLayout,
        vk::ImageLayout             newLayout,
        const VulkanCommandManager* commandManager,
        std::string&                errorMessage
    ) const;

private:
    vk::Image     _image{};
    vk::ImageView _imageView{};
    vk::Sampler   _sampler{};

    VmaAllocation _allocation{};

    vk::Format   _format{};
    vk::Extent3D _extent{};
};

#endif // NOBLEENGINE_VULKANIMAGE_H
