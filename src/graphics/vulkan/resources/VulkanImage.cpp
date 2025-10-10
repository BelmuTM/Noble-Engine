#include "VulkanImage.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

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

bool VulkanImage::copyBufferToImage(
    const vk::Buffer&  buffer,
    vk::Image&         image,
    const vk::Extent3D extent,
    std::string&       errorMessage
) const {
    vk::CommandBuffer copyCommandBuffer{};
    TRY(_commandManager->beginSingleTimeCommands(copyCommandBuffer, errorMessage));

    vk::BufferImageCopy region{};
    region
        .setBufferOffset(0)
        .setBufferRowLength(0)
        .setBufferImageHeight(0)
        .setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
        .setImageOffset({0, 0, 0})
        .setImageExtent(extent);

    copyCommandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, {region});

    TRY(_commandManager->endSingleTimeCommands(copyCommandBuffer, errorMessage));

    return true;
}

bool VulkanImage::transitionImageLayout(
    const vk::Image&      image,
    const vk::ImageLayout oldLayout,
    const vk::ImageLayout newLayout,
    std::string&          errorMessage
) const {
    vk::CommandBuffer copyCommandBuffer{};
    TRY(_commandManager->beginSingleTimeCommands(copyCommandBuffer, errorMessage));

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

    TRY(_commandManager->endSingleTimeCommands(copyCommandBuffer, errorMessage));

    return true;
}

bool VulkanImage::loadTextureFromFile(const char* path, std::string& errorMessage) {
    int depth = 1;

    int width, height, channels;
    stbi_uc* pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);

    if (!pixels) {
        errorMessage = "Failed to load texture \"" + std::string(path) + "\"";
        return false;
    }

    _extent = vk::Extent3D{static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(depth)};

    const vk::DeviceSize textureSize = width * height * channels;

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

    TRY(transitionImageLayout(_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, errorMessage));

    TRY(copyBufferToImage(stagingBuffer, _image, _extent, errorMessage));

    TRY(transitionImageLayout(
        _image,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        errorMessage
    ));

    return true;
}
