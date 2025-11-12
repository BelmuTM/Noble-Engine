#include "VulkanImage.h"

#include "graphics/vulkan/common/VulkanDebugger.h"
#include "graphics/vulkan/core/memory/VulkanBuffer.h"

#include "core/debug/ErrorHandling.h"

void VulkanImage::destroy(const VulkanDevice& device) noexcept {
    const vk::Device& logicalDevice = device.getLogicalDevice();
    VmaAllocator      allocator     = device.getAllocator();

    if (_sampler) {
        logicalDevice.destroySampler(_sampler);
        _sampler = VK_NULL_HANDLE;
    }

    if (_imageView) {
        logicalDevice.destroyImageView(_imageView);
        _imageView = VK_NULL_HANDLE;
    }

    if (allocator && _image) {
        vmaDestroyImage(allocator, _image, _allocation);
        _image      = VK_NULL_HANDLE;
        _allocation = VK_NULL_HANDLE;
    }
}

bool VulkanImage::createImage(
    const vk::Extent3D        extent,
    const vk::ImageType       type,
    const vk::Format          format,
    const vk::ImageUsageFlags usage,
    const VmaMemoryUsage      memoryUsage,
    const VulkanDevice*       device,
    std::string&              errorMessage
) {
    VmaAllocator allocator = device->getAllocator();

    vk::ImageCreateInfo imageInfo{};
    imageInfo
        .setImageType(type)
        .setFormat(format)
        .setExtent(extent)
        .setMipLevels(1)
        .setArrayLayers(1)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setTiling(vk::ImageTiling::eOptimal)
        .setUsage(usage)
        .setSharingMode(vk::SharingMode::eExclusive);

    VmaAllocationCreateInfo allocationInfo{};
    allocationInfo.usage = memoryUsage;

    VK_TRY(vmaCreateImage(allocator,
           reinterpret_cast<VkImageCreateInfo*>(&imageInfo),
           &allocationInfo,
           reinterpret_cast<VkImage*>(&_image),
           &_allocation,
           nullptr),
        errorMessage
    );

    return true;
}

bool VulkanImage::createImageView(
    const vk::ImageViewType    type,
    const vk::Format           format,
    const vk::ImageAspectFlags aspectFlags,
    const VulkanDevice*        device,
    std::string&               errorMessage
) {
    const vk::Device& logicalDevice = device->getLogicalDevice();

    vk::ImageViewCreateInfo imageViewInfo{};
    imageViewInfo
        .setImage(_image)
        .setViewType(type)
        .setFormat(format)
        .setSubresourceRange({aspectFlags, 0, 1, 0, 1});

    VK_CREATE(logicalDevice.createImageView(imageViewInfo), _imageView, errorMessage);

    return true;
}

bool VulkanImage::createSampler(
    const vk::Filter             filter,
    const vk::SamplerAddressMode addressMode,
    const VulkanDevice*          device,
    std::string&                 errorMessage
) {
    const vk::Device&         logicalDevice  = device->getLogicalDevice();
    const vk::PhysicalDevice& physicalDevice = device->getPhysicalDevice();

    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo
        .setMagFilter(filter)
        .setMinFilter(filter)
        .setMipmapMode(vk::SamplerMipmapMode::eLinear)
        .setAddressModeU(addressMode)
        .setAddressModeV(addressMode)
        .setAddressModeW(addressMode)
        .setMipLodBias(0.0f)
        .setAnisotropyEnable(vk::True)
        .setMaxAnisotropy(physicalDevice.getProperties().limits.maxSamplerAnisotropy)
        .setCompareEnable(vk::False)
        .setCompareOp(vk::CompareOp::eAlways)
        .setMinLod(0.0f)
        .setMaxLod(0.0f)
        .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
        .setUnnormalizedCoordinates(vk::False);

    VK_CREATE(logicalDevice.createSampler(samplerInfo), _sampler, errorMessage);

    return true;
}

bool VulkanImage::copyBufferToImage(
    const vk::Buffer&           buffer,
    const vk::Extent3D          extent,
    const VulkanCommandManager* commandManager,
    std::string&                errorMessage
) const {
    vk::CommandBuffer copyCommandBuffer{};
    TRY(commandManager->beginSingleTimeCommands(copyCommandBuffer, errorMessage));

    vk::BufferImageCopy region{};
    region
        .setBufferOffset(0)
        .setBufferRowLength(0)
        .setBufferImageHeight(0)
        .setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
        .setImageOffset({0, 0, 0})
        .setImageExtent(extent);

    copyCommandBuffer.copyBufferToImage(buffer, _image, vk::ImageLayout::eTransferDstOptimal, {region});

    TRY(commandManager->endSingleTimeCommands(copyCommandBuffer, errorMessage));

    return true;
}

bool VulkanImage::transitionImageLayout(
    const vk::ImageLayout       oldLayout,
    const vk::ImageLayout       newLayout,
    const VulkanCommandManager* commandManager,
    std::string&                errorMessage
) const {
    vk::CommandBuffer copyCommandBuffer{};
    TRY(commandManager->beginSingleTimeCommands(copyCommandBuffer, errorMessage));

    // Specify which part of the image to transition
    vk::ImageSubresourceRange subresourceRange{};
    subresourceRange
        .setBaseMipLevel(0)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

        if (hasStencilComponent(_format)) {
            subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
    } else {
        subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    }

    // Define how the transition operates and what to change
    vk::ImageMemoryBarrier barrier{};
    barrier
        .setOldLayout(oldLayout)
        .setNewLayout(newLayout)
        .setImage(_image)
        .setSubresourceRange(subresourceRange);

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {

        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage      = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;

    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
               newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {

        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage      = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;

    } else if (oldLayout == vk::ImageLayout::eUndefined &&
               newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead |
                                vk::AccessFlagBits::eDepthStencilAttachmentWrite;

        sourceStage      = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    } else {
        errorMessage = "Failed to transition Vulkan image layout: unsupported transition";
        return false;
    }

    copyCommandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);

    TRY(commandManager->endSingleTimeCommands(copyCommandBuffer, errorMessage));

    return true;
}

bool VulkanImage::createFromData(
    const void*                 pixels,
    const uint8_t               channels,
    const uint8_t               bytesPerChannel,
    const vk::Extent3D          extent,
    const vk::Format            format,
    const VulkanDevice*         device,
    const VulkanCommandManager* commandManager,
    std::string&                errorMessage
) {
    _extent = extent;
    _format = format;

    const vk::DeviceSize imageSize = _extent.width * _extent.height * channels * bytesPerChannel;

    VulkanBuffer stagingBuffer;

    TRY(stagingBuffer.create(
        imageSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        device,
        errorMessage
    ));

    void* stagingData = stagingBuffer.mapMemory(errorMessage);
    if (!stagingData) return false;

    memcpy(stagingData, pixels, imageSize);
    stagingBuffer.unmapMemory();

    TRY(createImage(
        _extent,
        vk::ImageType::e2D,
        format,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        VMA_MEMORY_USAGE_GPU_ONLY,
        device,
        errorMessage
    ));

    TRY(transitionImageLayout(
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal,
        commandManager,
        errorMessage
    ));

    TRY(copyBufferToImage(stagingBuffer, _extent, commandManager, errorMessage));

    TRY(transitionImageLayout(
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        commandManager,
        errorMessage
    ));

    TRY(createImageView(
        vk::ImageViewType::e2D,
        format,
        vk::ImageAspectFlagBits::eColor,
        device,
        errorMessage
    ));

    TRY(createSampler(vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, device, errorMessage));

    stagingBuffer.destroy();

    return true;
}
