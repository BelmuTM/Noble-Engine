#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/core/VulkanCommandManager.h"
#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/core/memory/VulkanBuffer.h"
#include "graphics/vulkan/resources/descriptors/VulkanDescriptorInfo.h"

#include "graphics/vulkan/core/memory/VmaUsage.h"

class VulkanImage {
public:
    VulkanImage()  = default;
    ~VulkanImage() = default;

    VulkanImage(const VulkanImage&)            = delete;
    VulkanImage& operator=(const VulkanImage&) = delete;

    VulkanImage(VulkanImage&&)            noexcept = default;
    VulkanImage& operator=(VulkanImage&&) noexcept = default;

    void destroy() noexcept;

    [[nodiscard]] Expected<void> transitionLayout(
        vk::CommandBuffer commandBuffer,
        vk::ImageLayout   oldLayout,
        vk::ImageLayout   newLayout,
        std::uint32_t     mipLevels = 1
    );

    [[nodiscard]] Expected<void> transitionLayout(
        const VulkanCommandManager* commandManager,
        vk::ImageLayout             oldLayout,
        vk::ImageLayout             newLayout,
        std::uint32_t               mipLevels = 1
    );

    [[nodiscard]] Expected<void> transitionLayout(
        vk::CommandBuffer commandBuffer,
        vk::ImageLayout   newLayout,
        std::uint32_t     mipLevels = 1
    );

    [[nodiscard]] Expected<void> transitionLayout(
        const VulkanCommandManager* commandManager,
        vk::ImageLayout             newLayout,
        std::uint32_t               mipLevels = 1
    );

    [[nodiscard]] Expected<void> createImage(
        vk::ImageType       type,
        vk::Format          format,
        vk::Extent3D        extent,
        std::uint32_t       mipLevels,
        vk::ImageUsageFlags usage,
        VmaMemoryUsage      memoryUsage,
        const VulkanDevice* device
    );

    [[nodiscard]] Expected<void> createImageView(
        vk::ImageViewType    type,
        vk::Format           format,
        vk::ImageAspectFlags aspectFlags,
        std::uint32_t        mipLevels,
        const VulkanDevice*  device
    );

    [[nodiscard]] Expected<void> createSampler(
        vk::Filter             filter,
        vk::SamplerAddressMode addressMode,
        const VulkanDevice*    device
    );

    void copyBufferToImage(
        vk::CommandBuffer commandBuffer,
        const vk::Buffer& buffer,
        vk::DeviceSize    offset
    ) const;

    void generateMipmaps(
        vk::CommandBuffer commandBuffer,
        vk::Extent3D      extent,
        std::uint32_t     mipLevels
    ) const;

    [[nodiscard]] Expected<void> createFromBuffer(
        const VulkanBuffer& buffer,
        vk::DeviceSize      bufferOffset,
        vk::Format          format,
        vk::Extent3D        extent,
        std::uint32_t       mipLevels,
        vk::CommandBuffer   commandBuffer,
        const VulkanDevice* device
    );

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

    [[nodiscard]] VulkanDescriptorInfo getDescriptorInfo(const std::uint32_t binding) const noexcept {
        return {
            .type      = _descriptorType,
            .imageInfo = {_sampler, _imageView, vk::ImageLayout::eShaderReadOnlyOptimal},
            .binding   = binding
        };
    }

    [[nodiscard]] vk::Image handle() const noexcept { return _image; }
    [[nodiscard]] vk::ImageView getImageView() const noexcept { return _imageView; }
    [[nodiscard]] vk::Sampler getSampler() const noexcept { return _sampler; }
    [[nodiscard]] vk::Format getFormat() const noexcept { return _format; }
    [[nodiscard]] vk::Extent3D getExtent() const noexcept { return _extent; }
    [[nodiscard]] vk::ImageUsageFlags getUsageFlags() const noexcept { return _usageFlags; }
    [[nodiscard]] vk::ImageAspectFlags getAspectFlags() const noexcept { return _aspectFlags; }
    [[nodiscard]] vk::ImageLayout getLayout() const noexcept { return _layout; }

    VulkanImage& setHandle(const vk::Image image) noexcept { _image = image; return *this; }
    VulkanImage& setImageView(const vk::ImageView imageView) noexcept { _imageView = imageView; return *this; }
    VulkanImage& setFormat(const vk::Format format) noexcept { _format = format; return *this; }
    VulkanImage& setUsageFlags(const vk::ImageUsageFlags usageFlags) noexcept {
        _usageFlags = usageFlags; return *this;
    }
    VulkanImage& setAspectFlags(const vk::ImageAspectFlags aspectFlags) noexcept {
        _aspectFlags = aspectFlags; return *this;
    }
    VulkanImage& setExtent(const vk::Extent3D extent) noexcept { _extent = extent; return *this; }
    VulkanImage& setLayout(const vk::ImageLayout layout) noexcept { _layout = layout; return *this; }
    VulkanImage& setDescriptorType(const vk::DescriptorType descriptorType) noexcept {
        _descriptorType = descriptorType; return *this;
    }

private:
    const VulkanDevice* _device = nullptr;

    vk::Image     _image{};
    vk::ImageView _imageView{};
    vk::Sampler   _sampler{};

    VmaAllocation _allocation = VK_NULL_HANDLE;

    vk::Format   _format = vk::Format::eUndefined;
    vk::Extent3D _extent{};

    vk::ImageUsageFlags  _usageFlags  = vk::ImageUsageFlagBits::eColorAttachment;
    vk::ImageAspectFlags _aspectFlags = vk::ImageAspectFlagBits::eColor;

    vk::ImageLayout _layout = vk::ImageLayout::eUndefined;

    vk::DescriptorType _descriptorType = vk::DescriptorType::eCombinedImageSampler;
};
