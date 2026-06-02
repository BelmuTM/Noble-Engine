#pragma once

#include "core/debug/ErrorHandling.h"

#include "core/resources/images/Image.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "VulkanImage.h"
#include "graphics/vulkan/core/VulkanCommandManager.h"

#include <mutex>
#include <unordered_map>

class VulkanImageManager {
public:
    // TODO: Eventually replace this with a function parameter
    // (fine for now since most images stored on the disk are RGBA8)
    static constexpr auto HARDCODED_IMAGE_FORMAT = vk::Format::eR8G8B8A8Unorm;

    static constexpr std::size_t STAGING_BUFFER_ALIGNMENT = 256ULL; // 256 bytes
    static constexpr std::size_t MAX_BATCH_SIZE = 64ULL * 1024U * 1024U; // 64 MB

    VulkanImageManager()  = default;
    ~VulkanImageManager() = default;

    VulkanImageManager(const VulkanImageManager&)            = delete;
    VulkanImageManager& operator=(const VulkanImageManager&) = delete;

    VulkanImageManager(VulkanImageManager&&)            = delete;
    VulkanImageManager& operator=(VulkanImageManager&&) = delete;

    [[nodiscard]] Expected<void> create(
        const VulkanDevice& device, const VulkanCommandManager& commandManager
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] Expected<void> loadImage(VulkanImage*& image, const Image* imageData);

    [[nodiscard]] Expected<void> loadImages(
        const std::vector<const Image*>& images,
        const VulkanBuffer&              stagingBuffer
    );

    [[nodiscard]] Expected<void> loadBatchedImages(const std::vector<const Image*>& images);

    [[nodiscard]] VulkanImage* getImage(const std::string& path) const {
        const auto cachedImage = _imageCache.find(path);
        return cachedImage != _imageCache.end() ? cachedImage->second.get() : nullptr;
    }

private:
    [[nodiscard]] static std::uint32_t getMipLevels(const vk::Extent3D extent) noexcept {
        return static_cast<std::uint32_t>(std::floor(std::log2(std::max({extent.width, extent.height, extent.depth})))) + 1;
    }

    const VulkanDevice*         _device         = nullptr;
    const VulkanCommandManager* _commandManager = nullptr;

    std::mutex _mutex{};

    std::unordered_map<std::string, std::unique_ptr<VulkanImage>> _imageCache{};
};
