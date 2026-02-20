#pragma once

#include "graphics/vulkan/core/VulkanCommandManager.h"
#include "graphics/vulkan/core/VulkanDevice.h"

#include "graphics/vulkan/resources/images/VulkanImage.h"

class VulkanRenderPassBufferFactory {
public:
    VulkanRenderPassBufferFactory()  = default;
    ~VulkanRenderPassBufferFactory() = default;

    [[nodiscard]] static bool createDepthBufferImage(
        VulkanImage&                depthBuffer,
        vk::Format                  format,
        vk::Extent2D                extent,
        const VulkanDevice*         device,
        const VulkanCommandManager* commandManager,
        std::string&                errorMessage
    );

    [[nodiscard]] static bool createColorBufferImage(
        VulkanImage&                colorBuffer,
        vk::Format                  format,
        vk::Extent2D                extent,
        const VulkanDevice*         device,
        const VulkanCommandManager* commandManager,
        std::string&                errorMessage
    );
};
