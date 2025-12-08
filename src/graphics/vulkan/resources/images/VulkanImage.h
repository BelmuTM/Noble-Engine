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

    VulkanImage(const VulkanImage&)            = delete;
    VulkanImage& operator=(const VulkanImage&) = delete;

    VulkanImage(VulkanImage&&)            = delete;
    VulkanImage& operator=(VulkanImage&&) = delete;

    void destroy() noexcept;

    [[nodiscard]] bool transitionLayout(
        vk::CommandBuffer commandBuffer,
        std::string&      errorMessage,
        vk::ImageLayout   oldLayout,
        vk::ImageLayout   newLayout,
        uint32_t          mipLevels = 1
    );

    [[nodiscard]] bool transitionLayout(
        const VulkanCommandManager* commandManager,
        std::string&                errorMessage,
        vk::ImageLayout             oldLayout,
        vk::ImageLayout             newLayout,
        uint32_t                    mipLevels = 1
    );

    [[nodiscard]] bool transitionLayout(
        vk::CommandBuffer commandBuffer,
        std::string&      errorMessage,
        vk::ImageLayout   newLayout,
        uint32_t          mipLevels = 1
    );

    [[nodiscard]] bool transitionLayout(
        const VulkanCommandManager* commandManager,
        std::string&                errorMessage,
        vk::ImageLayout             newLayout,
        uint32_t                    mipLevels = 1
    );

    [[nodiscard]] bool createImage(
        vk::ImageType       type,
        vk::Format          format,
        vk::Extent3D        extent,
        uint32_t            mipLevels,
        vk::ImageUsageFlags usage,
        VmaMemoryUsage      memoryUsage,
        const VulkanDevice* device,
        std::string&        errorMessage
    );

    [[nodiscard]] bool createImageView(
        vk::ImageViewType    type,
        vk::Format           format,
        vk::ImageAspectFlags aspectFlags,
        uint32_t             mipLevels,
        const VulkanDevice*  device,
        std::string&         errorMessage
    );

    [[nodiscard]] bool createSampler(
        vk::Filter             filter,
        vk::SamplerAddressMode addressMode,
        const VulkanDevice*    device,
        std::string&           errorMessage
    );

    [[nodiscard]] bool copyBufferToImage(
        const vk::Buffer&           buffer,
        vk::Extent3D                extent,
        const VulkanCommandManager* commandManager,
        std::string&                errorMessage
    ) const;

    [[nodiscard]] bool generateMipmaps(
        vk::Extent3D                extent,
        uint32_t                    mipLevels,
        const VulkanCommandManager* commandManager,
        std::string&                errorMessage
    ) const;

    [[nodiscard]] bool createFromData(
        const void*                 pixels,
        uint8_t                     channels,
        uint8_t                     bytesPerChannel,
        vk::Format                  format,
        vk::Extent3D                extent,
        uint32_t                    mipLevels,
        const VulkanDevice*         device,
        const VulkanCommandManager* commandManager,
        std::string&                errorMessage
    );

    [[nodiscard]] vk::ImageView getImageView() const noexcept { return _imageView; }
    [[nodiscard]] vk::Sampler getSampler() const noexcept { return _sampler; }
    [[nodiscard]] vk::Format getFormat() const noexcept { return _format; }
    [[nodiscard]] vk::Extent3D getExtent() const noexcept { return _extent; }
    [[nodiscard]] vk::ImageLayout getLayout() const noexcept { return _layout; }

    void setHandle(const vk::Image image) noexcept { _image = image; }
    void setImageView(const vk::ImageView imageView) noexcept { _imageView = imageView; }
    void setFormat(const vk::Format format) noexcept { _format = format; }
    void setExtent(const vk::Extent3D extent) noexcept { _extent = extent; }
    void setLayout(const vk::ImageLayout layout) noexcept { _layout = layout; }
    void setDescriptorType(const vk::DescriptorType descriptorType) noexcept { _descriptorType = descriptorType; }

    [[nodiscard]] static bool isDepthBuffer(const vk::Format format) {
        return format == vk::Format::eD16Unorm
            || format == vk::Format::eD32Sfloat
            || format == vk::Format::eD16UnormS8Uint
            || format == vk::Format::eD24UnormS8Uint
            || format == vk::Format::eD32SfloatS8Uint;
    }

    [[nodiscard]] static bool hasStencilComponent(const vk::Format format) {
        return format == vk::Format::eD16UnormS8Uint
            || format == vk::Format::eD24UnormS8Uint
            || format == vk::Format::eD32SfloatS8Uint;
    }

    [[nodiscard]] VulkanDescriptorInfo getDescriptorInfo(const uint32_t binding) const noexcept {
        return {
            .type      = _descriptorType,
            .imageInfo = {_sampler, _imageView, vk::ImageLayout::eShaderReadOnlyOptimal},
            .binding   = binding
        };
    }

private:
    const VulkanDevice* _device = nullptr;

    vk::Image     _image{};
    vk::ImageView _imageView{};
    vk::Sampler   _sampler{};

    VmaAllocation _allocation = VK_NULL_HANDLE;

    vk::Format   _format;
    vk::Extent3D _extent{};

    vk::ImageLayout _layout = vk::ImageLayout::eUndefined;

    vk::DescriptorType _descriptorType = vk::DescriptorType::eCombinedImageSampler;
};

#endif // NOBLEENGINE_VULKANIMAGE_H
