#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/core/VulkanCommandManager.h"
#include "graphics/vulkan/core/VulkanDevice.h"

#include "graphics/vulkan/resources/images/VulkanImage.h"

class VulkanRenderPassBufferFactory {
public:
    VulkanRenderPassBufferFactory()  = default;
    ~VulkanRenderPassBufferFactory() = default;

    [[nodiscard]] static Expected<void> createDepthBufferImage(
        VulkanImage&                depthBuffer,
        vk::Format                  format,
        vk::Extent2D                extent,
        const VulkanDevice*         device,
        const VulkanCommandManager* commandManager
    );

    [[nodiscard]] static Expected<void> createColorBufferImage(
        VulkanImage&                colorBuffer,
        vk::Format                  format,
        vk::Extent2D                extent,
        const VulkanDevice*         device,
        const VulkanCommandManager* commandManager
    );
};
