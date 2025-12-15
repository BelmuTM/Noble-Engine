#pragma once
#ifndef NOBLEENGINE_VULKANIMAGEMANAGER_H
#define NOBLEENGINE_VULKANIMAGEMANAGER_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "VulkanImage.h"
#include "graphics/vulkan/core/VulkanCommandManager.h"

#include "core/resources/images/Image.h"

#include <unordered_set>

class VulkanImageManager {
public:
    static constexpr size_t STAGING_BUFFER_ALIGNMENT = 256ULL; // 256 bytes
    static constexpr size_t MAX_BATCH_SIZE = 64ULL * 1024U * 1024U; // 64 MB

    VulkanImageManager()  = default;
    ~VulkanImageManager() = default;

    VulkanImageManager(const VulkanImageManager&)            = delete;
    VulkanImageManager& operator=(const VulkanImageManager&) = delete;

    VulkanImageManager(VulkanImageManager&&)            = delete;
    VulkanImageManager& operator=(VulkanImageManager&&) = delete;

    [[nodiscard]] bool create(
        const VulkanDevice& device, const VulkanCommandManager& commandManager, std::string& errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] bool loadImage(VulkanImage*& image, const Image* imageData, std::string& errorMessage);

    [[nodiscard]] bool loadImages(
        const std::vector<const Image*>& images,
        const VulkanBuffer&              stagingBuffer,
        std::string&                     errorMessage
    );

    [[nodiscard]] bool loadBatchedImages(const std::vector<const Image*>& images, std::string& errorMessage);

    [[nodiscard]] bool createColorBuffer(
        VulkanImage& colorBuffer, vk::Format format, vk::Extent2D extent, std::string& errorMessage
    ) const;

    [[nodiscard]] bool createDepthBuffer(
        VulkanImage& depthBuffer, vk::Extent2D extent, std::string& errorMessage
    ) const;

    [[nodiscard]] VulkanImage* getImage(const std::string& path) const {
        return _imageCache.contains(path) ? _imageCache.at(path).get() : nullptr;
    }

private:
    [[nodiscard]] static uint32_t getMipLevels(const vk::Extent3D extent) noexcept {
        return static_cast<uint32_t>(std::floor(std::log2(std::max({extent.width, extent.height, extent.depth})))) + 1;
    }

    const VulkanDevice*         _device         = nullptr;
    const VulkanCommandManager* _commandManager = nullptr;

    std::mutex _mutex{};

    std::unordered_map<std::string, std::unique_ptr<VulkanImage>> _imageCache{};
};

#endif //NOBLEENGINE_VULKANIMAGEMANAGER_H
