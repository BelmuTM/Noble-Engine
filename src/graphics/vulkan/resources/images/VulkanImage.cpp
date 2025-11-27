#include "VulkanImage.h"

#include "graphics/vulkan/common/VulkanDebugger.h"
#include "graphics/vulkan/core/memory/VulkanBuffer.h"
#include "VulkanImageLayoutTransitions.h"

#include "core/debug/ErrorHandling.h"

void VulkanImage::destroy(const VulkanDevice& device) noexcept {
    const vk::Device&   logicalDevice = device.getLogicalDevice();
    const VmaAllocator& allocator     = device.getAllocator();

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

bool VulkanImage::transitionLayout(
    const vk::CommandBuffer commandBuffer,
    std::string&            errorMessage,
    const vk::ImageLayout   oldLayout,
    const vk::ImageLayout   newLayout,
    const uint32_t          mipLevels
) {
    TRY(VulkanImageLayoutTransitions::transitionImageLayout(
        commandBuffer,
        errorMessage,
        _image,
        _format,
        oldLayout,
        newLayout,
        mipLevels
    ));

    _layout = newLayout;

    return true;
}

bool VulkanImage::transitionLayout(
    const VulkanCommandManager* commandManager,
    std::string&                errorMessage,
    const vk::ImageLayout       oldLayout,
    const vk::ImageLayout       newLayout,
    const uint32_t              mipLevels
) {
    vk::CommandBuffer commandBuffer{};
    TRY(commandManager->beginSingleTimeCommands(commandBuffer, errorMessage));

    TRY(transitionLayout(commandBuffer, errorMessage, oldLayout, newLayout, mipLevels));

    TRY(commandManager->endSingleTimeCommands(commandBuffer, errorMessage));

    return true;
}

bool VulkanImage::transitionLayout(
    const vk::CommandBuffer commandBuffer,
    std::string&            errorMessage,
    const vk::ImageLayout   newLayout,
    const uint32_t          mipLevels
) {
    TRY(VulkanImageLayoutTransitions::transitionImageLayout(
        commandBuffer,
        errorMessage,
        _image,
        _format,
        _layout,
        newLayout,
        mipLevels
    ));

    _layout = newLayout;

    return true;
}

bool VulkanImage::transitionLayout(
    const VulkanCommandManager* commandManager,
    std::string&                errorMessage,
    const vk::ImageLayout       newLayout,
    const uint32_t              mipLevels
) {
    vk::CommandBuffer commandBuffer{};
    TRY(commandManager->beginSingleTimeCommands(commandBuffer, errorMessage));

    TRY(transitionLayout(commandBuffer, errorMessage, newLayout, mipLevels));

    TRY(commandManager->endSingleTimeCommands(commandBuffer, errorMessage));

    return true;
}

bool VulkanImage::createImage(
    const vk::ImageType       type,
    const vk::Format          format,
    const vk::Extent3D        extent,
    const uint32_t            mipLevels,
    const vk::ImageUsageFlags usage,
    const VmaMemoryUsage      memoryUsage,
    const VulkanDevice*       device,
    std::string&              errorMessage
) {
    const VmaAllocator& allocator = device->getAllocator();

    vk::ImageCreateInfo imageInfo{};
    imageInfo
        .setImageType(type)
        .setFormat(format)
        .setExtent(extent)
        .setMipLevels(mipLevels)
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
    const uint32_t             mipLevels,
    const VulkanDevice*        device,
    std::string&               errorMessage
) {
    const vk::Device& logicalDevice = device->getLogicalDevice();

    vk::ImageSubresourceRange subresourceRange{};
    subresourceRange
        .setAspectMask(aspectFlags)
        .setBaseMipLevel(0)
        .setLevelCount(mipLevels)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    vk::ImageViewCreateInfo imageViewInfo{};
    imageViewInfo
        .setImage(_image)
        .setViewType(type)
        .setFormat(format)
        .setSubresourceRange(subresourceRange);

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
        .setMaxLod(vk::LodClampNone)
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

    vk::BufferImageCopy2 copyRegion{};
    copyRegion
        .setBufferOffset(0)
        .setBufferRowLength(0)
        .setBufferImageHeight(0)
        .setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
        .setImageOffset({0, 0, 0})
        .setImageExtent(extent);

    vk::CopyBufferToImageInfo2 copyBufferToImageInfo{};
    copyBufferToImageInfo
        .setSrcBuffer(buffer)
        .setDstImage(_image)
        .setDstImageLayout(vk::ImageLayout::eTransferDstOptimal)
        .setRegions({copyRegion});

    copyCommandBuffer.copyBufferToImage2(copyBufferToImageInfo);

    TRY(commandManager->endSingleTimeCommands(copyCommandBuffer, errorMessage));

    return true;
}

bool VulkanImage::generateMipmaps(
    const vk::Extent3D          extent,
    const uint32_t              mipLevels,
    const VulkanCommandManager* commandManager,
    std::string&                errorMessage
) const {
    if (mipLevels <= 1) return true;

    vk::CommandBuffer commandBuffer{};
    TRY(commandManager->beginSingleTimeCommands(commandBuffer, errorMessage));

    // Precompute mip sizes
    std::vector<vk::Extent3D> mipExtents(mipLevels);
    mipExtents[0] = extent;
    for (uint32_t i = 1; i < mipLevels; i++) {
        mipExtents[i].width  = std::max(1u, mipExtents[i - 1].width  / 2);
        mipExtents[i].height = std::max(1u, mipExtents[i - 1].height / 2);
        mipExtents[i].depth  = std::max(1u, mipExtents[i - 1].depth  / 2);
    }

    // Generate mips
    // Start from index 1 (full resolution level)
    for (uint32_t i = 1; i < mipLevels; i++) {
        // Transition level i - 1 to eTransferSrcOptimal
        // Waits for level i - 1 to be filled from previous vkCmdBlitImage
        vk::ImageSubresourceRange subresourceRange{};
        subresourceRange
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel(i - 1)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setLayerCount(1);

        auto oldLayout = vk::ImageLayout::eTransferDstOptimal;
        auto newLayout = vk::ImageLayout::eTransferSrcOptimal;

        auto transition = VulkanImageLayoutTransitions::getLayoutTransition(oldLayout, newLayout);
        vk::ImageMemoryBarrier2 barrierPreBlit{};
        barrierPreBlit
            .setSrcStageMask(transition->srcStage)
            .setSrcAccessMask(transition->srcAccessMask)
            .setDstStageMask(transition->dstStage)
            .setDstAccessMask(transition->dstAccessMask)
            .setOldLayout(oldLayout)
            .setNewLayout(newLayout)
            .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setImage(_image)
            .setSubresourceRange(subresourceRange);

        vk::DependencyInfo dependencyInfoPreBlit{};
        dependencyInfoPreBlit.setImageMemoryBarriers({barrierPreBlit});

        commandBuffer.pipelineBarrier2(dependencyInfoPreBlit);

        // Specify the source/destination regions targeted by the blit operation
        vk::ArrayWrapper1D<vk::Offset3D, 2> srcOffsets, dstOffsets;
        srcOffsets[0] = vk::Offset3D(0, 0, 0);
        srcOffsets[1] = vk::Offset3D(mipExtents[i - 1].width, mipExtents[i - 1].height, mipExtents[i - 1].depth);
        dstOffsets[0] = vk::Offset3D(0, 0, 0);
        dstOffsets[1] = vk::Offset3D(mipExtents[i].width, mipExtents[i].height, mipExtents[i].depth);

        vk::ImageBlit2 blit{};
        blit
            .setSrcSubresource({vk::ImageAspectFlagBits::eColor, i - 1, 0, 1})
            .setSrcOffsets(srcOffsets)
            .setDstSubresource({vk::ImageAspectFlagBits::eColor, i, 0, 1})
            .setDstOffsets(dstOffsets);

        vk::BlitImageInfo2 blitInfo{};
        blitInfo
            .setSrcImage(_image)
            .setSrcImageLayout(vk::ImageLayout::eTransferSrcOptimal)
            .setDstImage(_image)
            .setDstImageLayout(vk::ImageLayout::eTransferDstOptimal)
            .setRegions(blit)
            .setFilter(vk::Filter::eLinear);

        commandBuffer.blitImage2(blitInfo);

        // Transition level i - 1 to eShaderReadOnlyOptimal
        // Waits for the current vkCmdBlitImage to finish
        oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        transition = VulkanImageLayoutTransitions::getLayoutTransition(oldLayout, newLayout);
        vk::ImageMemoryBarrier2 barrierPostBlit{};
        barrierPostBlit
            .setSrcStageMask(transition->srcStage)
            .setSrcAccessMask(transition->srcAccessMask)
            .setDstStageMask(transition->dstStage)
            .setDstAccessMask(transition->dstAccessMask)
            .setOldLayout(oldLayout)
            .setNewLayout(newLayout)
            .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setImage(_image)
            .setSubresourceRange(subresourceRange);

        vk::DependencyInfo dependencyInfoPostBlit{};
        dependencyInfoPostBlit.setImageMemoryBarriers({barrierPostBlit});

        commandBuffer.pipelineBarrier2(dependencyInfoPostBlit);
    }

    // Transition last mip level to eShaderReadOnlyOptimal
    // The loop didn’t handle this, since the last mip level is never blitted from
    vk::ImageSubresourceRange subresourceRange{};
    subresourceRange
        .setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setBaseMipLevel(mipLevels - 1)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    auto oldLayout = vk::ImageLayout::eTransferDstOptimal;
    auto newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    auto transition = VulkanImageLayoutTransitions::getLayoutTransition(oldLayout, newLayout);
    vk::ImageMemoryBarrier2 barrier{};
    barrier
        .setSrcStageMask(transition->srcStage)
        .setSrcAccessMask(transition->srcAccessMask)
        .setDstStageMask(transition->dstStage)
        .setDstAccessMask(transition->dstAccessMask)
        .setOldLayout(oldLayout)
        .setNewLayout(newLayout)
        .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
        .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
        .setImage(_image)
        .setSubresourceRange(subresourceRange);

    vk::DependencyInfo dependencyInfo{};
    dependencyInfo.setImageMemoryBarriers({barrier});

    commandBuffer.pipelineBarrier2(dependencyInfo);

    TRY(commandManager->endSingleTimeCommands(commandBuffer, errorMessage));

    return true;
}

bool VulkanImage::createFromData(
    const void*                 pixels,
    const uint8_t               channels,
    const uint8_t               bytesPerChannel,
    const vk::Format            format,
    const vk::Extent3D          extent,
    const uint32_t              mipLevels,
    const VulkanDevice*         device,
    const VulkanCommandManager* commandManager,
    std::string&                errorMessage
) {
    _format         = format;
    _extent         = extent;
    _descriptorType = vk::DescriptorType::eCombinedImageSampler;

    const bool hasMipmaps = mipLevels > 1;

    const vk::DeviceSize imageSize = extent.width * extent.height * channels * bytesPerChannel;

    vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    if (hasMipmaps) {
        usageFlags |= vk::ImageUsageFlagBits::eTransferSrc;
    }

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

    std::memcpy(stagingData, pixels, imageSize);
    stagingBuffer.unmapMemory();

    TRY(createImage(
        vk::ImageType::e2D,
        format,
        extent,
        mipLevels,
        usageFlags,
        VMA_MEMORY_USAGE_GPU_ONLY,
        device,
        errorMessage
    ));

    TRY(transitionLayout(
        commandManager,
        errorMessage,
        vk::ImageLayout::eTransferDstOptimal,
        mipLevels
    ));

    TRY(copyBufferToImage(stagingBuffer, extent, commandManager, errorMessage));

    // Mipmaps generation
    if (hasMipmaps) {
        TRY(generateMipmaps(extent, mipLevels, commandManager, errorMessage));
    } else {
        TRY(transitionLayout(
            commandManager,
            errorMessage,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            mipLevels
        ));
    }

    TRY(createImageView(
        vk::ImageViewType::e2D,
        format,
        vk::ImageAspectFlagBits::eColor,
        mipLevels,
        device,
        errorMessage
    ));

    TRY(createSampler(vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, device, errorMessage));

    stagingBuffer.destroy();

    return true;
}
