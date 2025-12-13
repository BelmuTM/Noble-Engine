#include "VulkanImageManager.h"

#include "graphics/vulkan/core/memory/VulkanBuffer.h"

#include "core/debug/ErrorHandling.h"

#include <ranges>

bool VulkanImageManager::create(
    const VulkanDevice& device, const VulkanCommandManager& commandManager, std::string& errorMessage
) noexcept {
    _device         = &device;
    _commandManager = &commandManager;

    return true;
}

void VulkanImageManager::destroy() noexcept {
    for (const auto& image : _imageCache | std::views::values) {
        image->destroy();
    }

    _imageCache.clear();

    _device         = nullptr;
    _commandManager = nullptr;
}

bool VulkanImageManager::loadImage(
    VulkanImage*& image, const Image* imageData, const bool useMipmaps, std::string& errorMessage
) {
    if (!imageData) {
        errorMessage = "Failed to load Vulkan image: data is null";
        return false;
    }

    {
        // If image is already cached, return it
        std::lock_guard lock(_mutex);

        if (const auto it = _imageCache.find(imageData->path); it != _imageCache.end()) {
            image = it->second.get();
            return true;
        }
    }

    constexpr int depth = 1;

    const auto extent = vk::Extent3D{
        static_cast<uint32_t>(imageData->width),
        static_cast<uint32_t>(imageData->height),
        static_cast<uint32_t>(depth)
    };

    constexpr auto    format          = vk::Format::eR8G8B8A8Srgb;
    constexpr uint8_t bytesPerChannel = 1;

    const uint32_t mipLevels = useMipmaps ? getMipLevels(extent) : 1;

    VulkanImage tempImage{};

    // Create image
    TRY(tempImage.createFromData(
        imageData->pixels.data(),
        imageData->channels,
        bytesPerChannel,
        format,
        extent,
        mipLevels,
        _device,
        _commandManager,
        errorMessage
    ));

    {
        // Insert image into the cache
        std::lock_guard lock(_mutex);

        auto [it, inserted] = _imageCache.try_emplace(imageData->path, nullptr);

        if (inserted) {
            it->second = std::make_unique<VulkanImage>(std::move(tempImage));
        }

        image = it->second.get();
    }

    return true;
}

bool VulkanImageManager::loadBatchedImages(
    const std::vector<const Image*>& images, const bool useMipmaps, std::string& errorMessage
) {
    constexpr int depth = 1;

    constexpr auto format = vk::Format::eR8G8B8A8Srgb;

    vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    if (useMipmaps) {
        usageFlags |= vk::ImageUsageFlagBits::eTransferSrc;
    }

    std::vector<uint8_t> pixels{};

    vk::DeviceSize totalSize = 0;

    for (const auto& imageData : images) {
        pixels.insert(pixels.end(), imageData->pixels.begin(), imageData->pixels.end());

        totalSize += imageData->byteSize;
    }

    VulkanBuffer stagingBuffer;

    TRY(stagingBuffer.create(
        totalSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        _device,
        errorMessage
    ));

    void* stagingData = stagingBuffer.mapMemory(errorMessage);
    if (!stagingData) return false;

    std::memcpy(stagingData, pixels.data(), totalSize);

    stagingBuffer.unmapMemory();

    vk::CommandBuffer commandBuffer{};
    TRY(_commandManager->beginSingleTimeCommands(commandBuffer, errorMessage));

    vk::DeviceSize offset = 0;

    for (const auto& imageData : images) {
        if (!imageData) {
            errorMessage = "Failed to load Vulkan image: data is null";
            return false;
        }

        if (_imageCache.contains(imageData->path)) {
            continue;
        }

        const auto extent = vk::Extent3D{
            static_cast<uint32_t>(imageData->width),
            static_cast<uint32_t>(imageData->height),
            static_cast<uint32_t>(depth)
        };

        const uint32_t mipLevels = useMipmaps ? getMipLevels(extent) : 1;

        VulkanImage image{};

        image.setFormat(format);
        image.setExtent(extent);
        image.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);

        // Create image
        TRY(image.createImage(
            vk::ImageType::e2D,
            format,
            extent,
            mipLevels,
            usageFlags,
            VMA_MEMORY_USAGE_GPU_ONLY,
            _device,
            errorMessage
        ));

        TRY(image.transitionLayout(
            commandBuffer,
            errorMessage,
            vk::ImageLayout::eTransferDstOptimal,
            mipLevels
        ));

        image.copyBufferToImage(commandBuffer, stagingBuffer, offset);

        offset += imageData->byteSize;

        // Mipmaps generation
        if (useMipmaps) {
            image.generateMipmaps(commandBuffer, extent, mipLevels);

        } else {
            TRY(image.transitionLayout(
                commandBuffer,
                errorMessage,
                vk::ImageLayout::eShaderReadOnlyOptimal,
                mipLevels
            ));
        }

        TRY(image.createImageView(
            vk::ImageViewType::e2D,
            format,
            vk::ImageAspectFlagBits::eColor,
            mipLevels,
            _device,
            errorMessage
        ));

        TRY(image.createSampler(vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, _device, errorMessage));

        {
            // Insert image into the cache
            std::lock_guard lock(_mutex);

            _imageCache.emplace(imageData->path, std::make_unique<VulkanImage>(std::move(image)));
        }
    }

    TRY(_commandManager->endSingleTimeCommands(commandBuffer, errorMessage));

    stagingBuffer.destroy();

    return true;
}

bool VulkanImageManager::createColorBuffer(
    VulkanImage& colorBuffer, const vk::Format format, const vk::Extent2D extent, std::string& errorMessage
) const {
    const auto colorExtent = vk::Extent3D{extent.width, extent.height, 1};

    colorBuffer.setFormat(format);
    colorBuffer.setExtent(colorExtent);
    colorBuffer.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);

    TRY(colorBuffer.createImage(
        vk::ImageType::e2D,
        format,
        colorExtent,
        1,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
        VMA_MEMORY_USAGE_GPU_ONLY,
        _device,
        errorMessage
    ));

    TRY(colorBuffer.createImageView(
        vk::ImageViewType::e2D, format, vk::ImageAspectFlagBits::eColor, 1, _device, errorMessage
    ));

    TRY(colorBuffer.transitionLayout(
        _commandManager, errorMessage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal
    ));

    TRY(colorBuffer.createSampler(vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, _device, errorMessage));

    return true;
}

bool VulkanImageManager::createDepthBuffer(
    VulkanImage& depthBuffer, const vk::Extent2D extent, std::string& errorMessage
) const {
    constexpr auto depthFormat = vk::Format::eD32Sfloat;
    const     auto depthExtent = vk::Extent3D(extent.width, extent.height, 1);

    depthBuffer.setFormat(depthFormat);
    depthBuffer.setExtent(depthExtent);
    depthBuffer.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);

    vk::ImageAspectFlags aspects = vk::ImageAspectFlagBits::eDepth;
    if (VulkanImage::hasStencilComponent(depthBuffer.getFormat())) {
        aspects |= vk::ImageAspectFlagBits::eStencil;
    }

    TRY(depthBuffer.createImage(
        vk::ImageType::e2D,
        depthFormat,
        depthExtent,
        1,
        vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
        VMA_MEMORY_USAGE_GPU_ONLY,
        _device,
        errorMessage
    ));

    TRY(depthBuffer.createImageView(vk::ImageViewType::e2D, depthFormat, aspects, 1, _device, errorMessage));

    TRY(depthBuffer.transitionLayout(
        _commandManager, errorMessage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilAttachmentOptimal
    ));

    TRY(depthBuffer.createSampler(vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, _device, errorMessage));

    return true;
}
