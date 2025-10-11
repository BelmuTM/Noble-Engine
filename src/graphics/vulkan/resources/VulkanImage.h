#pragma once
#ifndef NOBLEENGINE_VULKANIMAGE_H
#define NOBLEENGINE_VULKANIMAGE_H

#include "graphics/vulkan/common/VulkanHeader.h"
#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/core/memory/VulkanBuffer.h"
#include "graphics/vulkan/resources/VulkanDescriptor.h"

#include "graphics/vulkan/core/memory/VmaUsage.h"

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

    [[nodiscard]] static bool createImageView(
        vk::ImageView&      imageView,
        const vk::Image&    image,
        vk::ImageViewType   type,
        vk::Format          format,
        const VulkanDevice* device,
        std::string&        errorMessage
    );

    [[nodiscard]] static bool createSampler(
        vk::Sampler&           sampler,
        vk::Filter             filter,
        vk::SamplerAddressMode addressMode,
        const VulkanDevice*    device,
        std::string&           errorMessage
    );

    void bindToDescriptor(
        const VulkanDescriptor& descriptor,
        uint32_t                binding,
        uint32_t                framesInFlight
    ) const;

    [[nodiscard]] static bool copyBufferToImage(
        const vk::Buffer&           buffer,
        const vk::Image&            image,
        vk::Extent3D                extent,
        const VulkanCommandManager* commandManager,
        std::string&                errorMessage
    );

    [[nodiscard]] static bool transitionImageLayout(
        const vk::Image&            image,
        vk::ImageLayout             oldLayout,
        vk::ImageLayout             newLayout,
        const VulkanCommandManager* commandManager,
        std::string&                errorMessage
    );

    [[nodiscard]] bool loadTextureFromFile(const char* path, std::string& errorMessage);

private:
    const VulkanDevice*         _device         = nullptr;
    const VulkanCommandManager* _commandManager = nullptr;

    vk::Image     _image{};
    vk::ImageView _imageView{};
    vk::Sampler   _sampler{};

    VmaAllocation _allocation{};

    vk::Extent3D _extent{};
};

#endif // NOBLEENGINE_VULKANIMAGE_H
