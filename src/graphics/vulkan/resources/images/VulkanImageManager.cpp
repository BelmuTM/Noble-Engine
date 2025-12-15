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

bool VulkanImageManager::loadImage(VulkanImage*& image, const Image* imageData, std::string& errorMessage) {
    if (!imageData) {
        errorMessage = "Failed to load Vulkan image: image is null";
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

    constexpr auto format  = vk::Format::eR8G8B8A8Srgb;

    const uint32_t mipLevels = imageData->hasMipmaps ? getMipLevels(extent) : 1;

    VulkanBuffer stagingBuffer;
    // Create the staging buffer
    TRY(stagingBuffer.create(
        imageData->byteSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        _device,
        errorMessage
    ));

    void* stagingData = stagingBuffer.mapMemory(errorMessage);

    if (!stagingData) {
        errorMessage = "Failed to create Vulkan image staging buffer: mapped memory pointer is null";
        return false;
    }

    // Copy the image's bytes into the staging buffer
    std::memcpy(stagingData, imageData->pixels.data(), imageData->byteSize);

    stagingBuffer.unmapMemory();

    // Create image on the GPU
    vk::CommandBuffer commandBuffer{};
    TRY(_commandManager->beginSingleTimeCommands(commandBuffer, errorMessage));

    VulkanImage tempImage{};
    TRY(tempImage.createFromBuffer(stagingBuffer, 0, format, extent, mipLevels, commandBuffer, _device, errorMessage));

    TRY(_commandManager->endSingleTimeCommands(commandBuffer, errorMessage));

    stagingBuffer.destroy();

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

bool VulkanImageManager::loadImages(
    const std::vector<const Image*>& images, const VulkanBuffer& stagingBuffer, std::string& errorMessage
) {
    if (images.empty()) return true;

    constexpr auto format = vk::Format::eR8G8B8A8Srgb;

    constexpr int depth = 1;

    vk::CommandBuffer commandBuffer{};
    TRY(_commandManager->beginSingleTimeCommands(commandBuffer, errorMessage));

    vk::DeviceSize offset = 0;

    for (const Image* image : images) {
        if (!image) {
            errorMessage = "Failed to load Vulkan image: image is null";
            return false;
        }

        if (_imageCache.contains(image->path)) {
            continue;
        }

        const auto extent = vk::Extent3D{
            static_cast<uint32_t>(image->width),
            static_cast<uint32_t>(image->height),
            static_cast<uint32_t>(depth)
        };

        const uint32_t mipLevels = image->hasMipmaps ? getMipLevels(extent) : 1;

        // Ensure image data is aligned properly in memory
        offset = (offset + STAGING_BUFFER_ALIGNMENT - 1) & ~(STAGING_BUFFER_ALIGNMENT - 1);

        // Copy the image's bytes into the staging buffer
        std::memcpy(static_cast<char*>(stagingBuffer.getMappedPointer()) + offset, image->pixels.data(), image->byteSize);

        // Create image on the GPU
        VulkanImage tempImage{};
        TRY(tempImage.createFromBuffer(
            stagingBuffer, offset, format, extent, mipLevels, commandBuffer, _device, errorMessage
        ));

        offset += image->byteSize;

        _imageCache.emplace(image->path, std::make_unique<VulkanImage>(std::move(tempImage)));
    }

    TRY(_commandManager->endSingleTimeCommands(commandBuffer, errorMessage));

    return true;
}

bool VulkanImageManager::loadBatchedImages(const std::vector<const Image*>& images, std::string& errorMessage) {
    if (images.empty()) return true;

    VulkanBuffer stagingBuffer;
    // Create the staging buffer
    TRY(stagingBuffer.create(
        MAX_BATCH_SIZE,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        _device,
        errorMessage
    ));

    const void* stagingData = stagingBuffer.mapMemory(errorMessage);

    if (!stagingData) {
        errorMessage = "Failed to create Vulkan image staging buffer: mapped memory pointer is null";
        return false;
    }

    std::vector<const Image*> batch{};

    size_t currentBatchSize = 0;

    for (const Image* image : images) {
        if (!image) {
            errorMessage = "Failed to batch Vulkan image: image is null";
            return false;
        }

        if (_imageCache.contains(image->path)) {
            continue;
        }

        // If image alone is larger than the maximum batch size (special case, rare)
        if (image->byteSize > MAX_BATCH_SIZE) {
            // Flush batched images
            if (!batch.empty()) {
                TRY(loadImages(batch, stagingBuffer, errorMessage));
                batch.clear();
            }

            currentBatchSize = 0;

            // Load image with its own dedicated staging buffer
            VulkanImage* _;
            TRY(loadImage(_, image, errorMessage));

            continue;
        }

        // If adding image to the batch exceeds the maximum batch size
        if (currentBatchSize + image->byteSize > MAX_BATCH_SIZE) {
            // Flush batched images
            TRY(loadImages(batch, stagingBuffer, errorMessage));
            batch.clear();

            currentBatchSize = 0;
        }

        batch.push_back(image);

        currentBatchSize += image->byteSize;
    }

    // Flush remaining batched images
    if (!batch.empty()) {
        TRY(loadImages(batch, stagingBuffer, errorMessage));
    }

    batch.clear();

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
