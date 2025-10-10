#pragma once
#ifndef NOBLEENGINE_VULKANIMAGE_H
#define NOBLEENGINE_VULKANIMAGE_H

#include "graphics/vulkan/common/VulkanHeader.h"
#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/core/memory/VulkanBuffer.h"

#include "graphics/vulkan/core/memory/VmaUsage.h"
#include "graphics/vulkan/resources/StbUsage.h"

class VulkanImage {
public:
    VulkanImage()  = default;
    ~VulkanImage() = default;

    VulkanImage(const VulkanImage&)            = delete;
    VulkanImage& operator=(const VulkanImage&) = delete;
    VulkanImage(VulkanImage&&)                 = delete;
    VulkanImage& operator=(VulkanImage&&)      = delete;

    [[nodiscard]] bool create(
        const VulkanDevice&         device,
        const VulkanCommandManager& commandManager,
        std::string&                errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] static bool createImage(
        vk::Image&          image,
        VmaAllocation&      allocation,
        vk::Extent3D        extent,
        vk::ImageType       type,
        vk::Format          format,
        vk::ImageUsageFlags usage,
        VmaMemoryUsage      memoryUsage,
        const VulkanDevice* device,
        std::string&        errorMessage
    );

    [[nodiscard]] bool loadTextureFromFile(const char* path, std::string& errorMessage);

private:
    const VulkanDevice*         _device         = nullptr;
    const VulkanCommandManager* _commandManager = nullptr;

    vk::Image     _image{};
    VmaAllocation _allocation{};

    vk::Extent3D _extent{};

    bool copyBufferToImage(
        const vk::Buffer& buffer,
        vk::Image&        image,
        vk::Extent3D      extent,
        std::string&      errorMessage
    ) const;

    [[nodiscard]] bool transitionImageLayout(
        const vk::Image& image,
        vk::ImageLayout  oldLayout,
        vk::ImageLayout  newLayout,
        std::string&     errorMessage
    ) const;
};

#endif // NOBLEENGINE_VULKANIMAGE_H
