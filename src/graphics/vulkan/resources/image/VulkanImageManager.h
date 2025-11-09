#pragma once
#ifndef NOBLEENGINE_VULKANIMAGEMANAGER_H
#define NOBLEENGINE_VULKANIMAGEMANAGER_H

#include "graphics/vulkan/common/VulkanHeader.h"
#include "VulkanImage.h"
#include "graphics/vulkan/core/VulkanCommandManager.h"

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

    void addImage(const VulkanImage& image) {
        _images.push_back(image);
    }

    [[nodiscard]] bool createDefaultTexture(VulkanImage& texture, std::string& errorMessage);

    [[nodiscard]] bool loadTextureFromFile(VulkanImage& texture, const std::string& path, std::string& errorMessage);

    [[nodiscard]] bool createColorBuffer(
        VulkanImage& colorBuffer, vk::Extent2D extent, vk::Format format, std::string& errorMessage
    ) const;

    [[nodiscard]] bool createDepthBuffer(
        VulkanImage& depthBuffer, vk::Extent2D extent, std::string& errorMessage
    ) const;

private:
    const VulkanDevice*         _device         = nullptr;
    const VulkanCommandManager* _commandManager = nullptr;

    std::vector<VulkanImage> _images{};
};

#endif //NOBLEENGINE_VULKANIMAGEMANAGER_H
