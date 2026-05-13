#include "VulkanImageManager.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/core/memory/VulkanBuffer.h"

#include <ranges>

Expected<void> VulkanImageManager::create(
    const VulkanDevice& device, const VulkanCommandManager& commandManager
) noexcept {
    _device         = &device;
    _commandManager = &commandManager;

    return {};
}

void VulkanImageManager::destroy() noexcept {
    for (const auto& image : _imageCache | std::views::values) {
        image->destroy();
    }

    _imageCache.clear();

    _device         = nullptr;
    _commandManager = nullptr;
}

Expected<void> VulkanImageManager::loadImage(VulkanImage*& image, const Image* imageData) {
    if (!imageData) {
        return VK_FAIL("Failed to upload image: image is null.");
    }

    {
        // Fast path: image already in cache
        std::lock_guard lock(_mutex);

        if (const auto it = _imageCache.find(imageData->path); it != _imageCache.end()) {
            image = it->second.get();
            return {};
        }
    }

    constexpr int depth = 1;

    const auto extent = vk::Extent3D{
        static_cast<std::uint32_t>(imageData->width),
        static_cast<std::uint32_t>(imageData->height),
        static_cast<std::uint32_t>(depth)
    };

    constexpr auto format = HARDCODED_IMAGE_FORMAT;

    const std::uint32_t mipLevels = imageData->hasMipmaps ? getMipLevels(extent) : 1;

    VulkanBuffer stagingBuffer;
    // Create the staging buffer
    TRY(stagingBuffer.create(
        imageData->byteSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        _device
    ));

    TRY(stagingBuffer.mapMemory());

    // Copy the image's bytes into the staging buffer
    stagingBuffer.updateMemory(imageData->pixels.get(), imageData->byteSize);

    stagingBuffer.unmapMemory();

    // Create image on the GPU
    vk::CommandBuffer commandBuffer{};
    TRY(_commandManager->beginSingleTimeCommands(commandBuffer));

    VulkanImage tempImage{};
    TRY(tempImage.createFromBuffer(stagingBuffer, 0, format, extent, mipLevels, commandBuffer, _device));

    TRY(_commandManager->endSingleTimeCommands(commandBuffer));

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

    return {};
}

Expected<void> VulkanImageManager::loadImages(
    const std::vector<const Image*>& images, const VulkanBuffer& stagingBuffer
) {
    if (images.empty()) return {};

    constexpr auto format = HARDCODED_IMAGE_FORMAT;

    constexpr int depth = 1;

    vk::CommandBuffer commandBuffer{};
    TRY(_commandManager->beginSingleTimeCommands(commandBuffer));

    vk::DeviceSize offset = 0;

    for (const Image* image : images) {
        if (!image) {
            return VK_FAIL("Failed to upload image: image is null.");
        }

        if (_imageCache.contains(image->path)) {
            continue;
        }

        const auto extent = vk::Extent3D{
            static_cast<std::uint32_t>(image->width),
            static_cast<std::uint32_t>(image->height),
            static_cast<std::uint32_t>(depth)
        };

        const std::uint32_t mipLevels = image->hasMipmaps ? getMipLevels(extent) : 1;

        // Ensure image data is aligned properly in memory
        offset = VulkanBuffer::align(offset, STAGING_BUFFER_ALIGNMENT);

        // Copy the image's bytes into the staging buffer
        stagingBuffer.updateMemory(image->pixels.get(), image->byteSize, offset);

        // Create image on the GPU
        VulkanImage tempImage{};
        TRY(tempImage.createFromBuffer(stagingBuffer, offset, format, extent, mipLevels, commandBuffer, _device));

        offset += image->byteSize;

        _imageCache.emplace(image->path, std::make_unique<VulkanImage>(std::move(tempImage)));
    }

    TRY(_commandManager->endSingleTimeCommands(commandBuffer));

    return {};
}

// TODO: Make this multithreaded.
Expected<void> VulkanImageManager::loadBatchedImages(const std::vector<const Image*>& images) {
    if (images.empty()) return {};

    VulkanBuffer stagingBuffer;
    // Create the staging buffer
    TRY(stagingBuffer.create(
        MAX_BATCH_SIZE,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        _device
    ));

    TRY(stagingBuffer.mapMemory());

    std::vector<const Image*> batch{};

    std::size_t currentBatchSize = 0;

    for (const Image* image : images) {
        if (!image) {
            return VK_FAIL("Failed to batch image for upload: image is null.");
        }

        if (_imageCache.contains(image->path)) {
            continue;
        }

        // If image alone is larger than the maximum batch size (special case, rare)
        if (image->byteSize > MAX_BATCH_SIZE) {
            // Flush batched images
            if (!batch.empty()) {
                TRY(loadImages(batch, stagingBuffer));
                batch.clear();
            }

            currentBatchSize = 0;

            // Load image with its own dedicated staging buffer
            VulkanImage* _ = nullptr;
            TRY(loadImage(_, image));

            continue;
        }

        // If adding image to the batch exceeds the maximum batch size
        if (currentBatchSize + image->byteSize > MAX_BATCH_SIZE) {
            // Flush batched images
            TRY(loadImages(batch, stagingBuffer));
            batch.clear();

            currentBatchSize = 0;
        }

        batch.push_back(image);

        currentBatchSize += image->byteSize;
    }

    // Flush remaining batched images
    if (!batch.empty()) {
        TRY(loadImages(batch, stagingBuffer));
    }

    batch.clear();

    stagingBuffer.destroy();

    return {};
}
