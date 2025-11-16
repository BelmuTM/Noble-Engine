#pragma once
#ifndef NOBLEENGINE_VULKANIMAGEMANAGER_H
#define NOBLEENGINE_VULKANIMAGEMANAGER_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "VulkanImage.h"
#include "graphics/vulkan/core/VulkanCommandManager.h"

#include "core/resources/images/Image.h"

class VulkanImageManager {
public:
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

    [[nodiscard]] bool loadImage(
        VulkanImage*& image, const Image* imageData, bool useMipmaps, std::string& errorMessage
    );

    [[nodiscard]] bool createColorBuffer(
        VulkanImage& colorBuffer, vk::Format format, vk::Extent2D extent, std::string& errorMessage
    ) const;

    [[nodiscard]] bool createDepthBuffer(
        VulkanImage& depthBuffer, vk::Extent2D extent, std::string& errorMessage
    ) const;

private:
    const VulkanDevice*         _device         = nullptr;
    const VulkanCommandManager* _commandManager = nullptr;

    std::vector<std::unique_ptr<VulkanImage>> _images{};

    [[nodiscard]] static uint32_t getMipLevels(const vk::Extent3D extent) {
        return static_cast<uint32_t>(std::floor(std::log2(std::max({extent.width, extent.height, extent.depth})))) + 1;
    }
};

#endif //NOBLEENGINE_VULKANIMAGEMANAGER_H
