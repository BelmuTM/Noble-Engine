#pragma once
#ifndef NOBLEENGINE_VULKANRENDERER_H
#define NOBLEENGINE_VULKANRENDERER_H

#include "graphics/GraphicsAPI.h"
#include "graphics/vulkan/common/VulkanEntityOwner.h"

#include "core/platform/Platform.h"

#include "VulkanContext.h"
#include "VulkanCommandManager.h"
#include "VulkanSyncObjects.h"
#include "graphics/vulkan/pipeline/VulkanGraphicsPipeline.h"
#include "graphics/vulkan/resources/VulkanMesh.h"
#include "graphics/vulkan/resources/VulkanMeshManager.h"

class VulkanRenderer final : public GraphicsAPI, public VulkanEntityOwner<VulkanRenderer> {
public:
    VulkanRenderer() = default;
    ~VulkanRenderer() override;

    bool init(Platform::Window& window) override;
    void shutdown() override;
    void drawFrame() override;

private:
    Platform::Window* _window = nullptr;

    VulkanContext          context;
    VulkanCommandManager   commandManager;
    VulkanSyncObjects      syncObjects;
    VulkanGraphicsPipeline graphicsPipeline;
    VulkanMesh             mesh;
    VulkanMeshManager      meshManager;

    unsigned int currentFrame = 0;

    bool handleFramebufferResize(std::string& errorMessage);

    std::optional<uint32_t> acquireNextImage(std::string& errorMessage, bool& discardLogging);

    void waitForImageFence(uint32_t imageIndex);

    void recordCurrentCommandBuffer(uint32_t imageIndex);
    bool submitCommandBuffer(uint32_t imageIndex, std::string& errorMessage, bool& discardLogging);

    void transitionImageLayout(
        vk::CommandBuffer       commandBuffer,
        uint32_t                imageIndex,
        vk::ImageLayout         oldLayout,
        vk::ImageLayout         newLayout,
        vk::AccessFlags2        srcAccessMask,
        vk::AccessFlags2        dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask
    ) const;

    static bool beginCommandBuffer(vk::CommandBuffer commandBuffer, std::string& errorMessage) ;
    void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex) const;

    bool recreateSwapchain(std::string& errorMessage);

    void updateUniformBuffer();
};

#endif //NOBLEENGINE_VULKANRENDERER_H
