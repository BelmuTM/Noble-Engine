#pragma once
#ifndef NOBLEENGINE_VULKANRENDERER_H
#define NOBLEENGINE_VULKANRENDERER_H

#include "graphics/GraphicsAPI.h"
#include "graphics/vulkan/common/VulkanEntityOwner.h"

#include "core/platform/Platform.h"

#include "VulkanContext.h"
#include "VulkanCommandManager.h"
#include "VulkanSwapchainManager.h"
#include "graphics/vulkan/resources/VulkanDescriptor.h"
#include "graphics/vulkan/pipeline/VulkanGraphicsPipeline.h"
#include "graphics/vulkan/resources/VulkanUniformBuffers.h"
#include "graphics/vulkan/resources/VulkanMeshManager.h"
#include "graphics/vulkan/pipeline/VulkanFrameGraph.h"

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
    VulkanSwapchainManager swapchainManager;
    VulkanDescriptor       descriptor;
    VulkanGraphicsPipeline pipeline;

    VulkanUniformBuffers<UniformBufferObject> uniformBuffers;

    VulkanMeshManager meshManager;

    VulkanFrameGraph frameGraph;

    unsigned int currentFrame = 0;

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

    static bool beginCommandBuffer(vk::CommandBuffer commandBuffer, std::string& errorMessage);

    void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
    void recordCurrentCommandBuffer(uint32_t imageIndex);

    void updateUniformBuffer();
};

#endif //NOBLEENGINE_VULKANRENDERER_H
