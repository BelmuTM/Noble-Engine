#include "VulkanImage.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/resources/StbUsage.h"

#include "core/debug/ErrorHandling.h"

bool VulkanImage::create(
    const VulkanDevice&         device,
    const VulkanCommandManager& commandManager,
    std::string&                errorMessage
) noexcept {
    _device         = &device;
    _commandManager = &commandManager;

    return true;
}

void VulkanImage::destroy() noexcept {
    if (!_device) return;

    const vk::Device& logicalDevice = _device->getLogicalDevice();
    VmaAllocator      allocator     = _device->getAllocator();

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

    _device         = nullptr;
    _commandManager = nullptr;
}

bool VulkanImage::createImage(
    vk::Image&                image,
    VmaAllocation&            allocation,
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
           reinterpret_cast<VkImage*>(&image),
           &allocation,
           nullptr),
        errorMessage
    );

    return true;
}

bool VulkanImage::createImageView(
    vk::ImageView&          imageView,
    const vk::Image&        image,
    const vk::ImageViewType type,
    const vk::Format        format,
    const VulkanDevice*     device,
    std::string&            errorMessage
) {
    const vk::Device& logicalDevice = device->getLogicalDevice();

    vk::ImageViewCreateInfo imageViewInfo{};
    imageViewInfo
        .setImage(image)
        .setViewType(type)
        .setFormat(format)
        .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

    VK_CREATE(logicalDevice.createImageView(imageViewInfo), imageView, errorMessage);

    return true;
}

bool VulkanImage::copyBufferToImage(
    const vk::Buffer&           buffer,
    const vk::Image&            image,
    const vk::Extent3D          extent,
    const VulkanCommandManager* commandManager,
    std::string&                errorMessage
) {
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

    copyCommandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, {region});

    TRY(commandManager->endSingleTimeCommands(copyCommandBuffer, errorMessage));

    return true;
}

bool VulkanImage::createSampler(
    vk::Sampler&                 sampler,
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

    VK_CREATE(logicalDevice.createSampler(samplerInfo), sampler, errorMessage);

    return true;
}

void VulkanImage::bindToDescriptor(const VulkanDescriptor& descriptor, const uint32_t binding) const {
    vk::DescriptorImageInfo descriptorImageInfo{};
    descriptorImageInfo
        .setSampler(_sampler)
        .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setImageView(_imageView);

    descriptor.updateSets(binding, vk::DescriptorType::eCombinedImageSampler, descriptorImageInfo);
}

bool VulkanImage::transitionImageLayout(
    const vk::Image&            image,
    const vk::ImageLayout       oldLayout,
    const vk::ImageLayout       newLayout,
    const VulkanCommandManager* commandManager,
    std::string&                errorMessage
) {
    vk::CommandBuffer copyCommandBuffer{};
    TRY(commandManager->beginSingleTimeCommands(copyCommandBuffer, errorMessage));

    // Specify which part of the image to transition
    vk::ImageSubresourceRange subresourceRange{};
    subresourceRange
        .setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setBaseMipLevel(0)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    // Define how the transition operates and what to change
    vk::ImageMemoryBarrier barrier{};
    barrier
        .setOldLayout(oldLayout)
        .setNewLayout(newLayout)
        .setImage(image)
        .setSubresourceRange(subresourceRange);

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {

        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;

    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
               newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {

        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage      = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;

    } else {
        errorMessage = "Failed to transition Vulkan image layout: unsupported transition";
        return false;
    }

    copyCommandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);

    TRY(commandManager->endSingleTimeCommands(copyCommandBuffer, errorMessage));

    return true;
}

bool VulkanImage::loadTextureFromFile(const char* path, std::string& errorMessage) {
    constexpr int depth = 1;

    int width, height, channels;
    stbi_uc* pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);

    if (!pixels) {
        errorMessage = "Failed to load texture \"" + std::string(path) + "\"";
        return false;
    }

    _extent = vk::Extent3D{static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(depth)};

    const vk::DeviceSize textureSize = width * height * STBI_rgb_alpha;

    VulkanBuffer stagingBuffer;

    TRY(stagingBuffer.create(
        textureSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        _device,
        errorMessage
    ));

    void* stagingData = stagingBuffer.mapMemory(errorMessage);
    if (!stagingData) return false;

    memcpy(stagingData, pixels, textureSize);
    stagingBuffer.unmapMemory();

    stbi_image_free(pixels);

    TRY(createImage(
        _image,
        _allocation,
        _extent,
        vk::ImageType::e2D,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        _device,
        errorMessage
    ));

    TRY(transitionImageLayout(
        _image,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal,
        _commandManager,
        errorMessage
    ));

    TRY(copyBufferToImage(stagingBuffer, _image, _extent, _commandManager, errorMessage));

    TRY(transitionImageLayout(
        _image,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        _commandManager,
        errorMessage
    ));

    TRY(createImageView(_imageView, _image, vk::ImageViewType::e2D, vk::Format::eR8G8B8A8Srgb, _device, errorMessage));

    TRY(createSampler(_sampler, vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, _device, errorMessage));

    stagingBuffer.destroy();

    return true;
}
