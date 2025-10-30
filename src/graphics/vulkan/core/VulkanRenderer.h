#pragma once
#ifndef NOBLEENGINE_VULKANRENDERER_H
#define NOBLEENGINE_VULKANRENDERER_H

#include "graphics/GraphicsAPI.h"
#include "graphics/vulkan/common/VulkanEntityOwner.h"

#include "core/Platform.h"

#include "VulkanContext.h"
#include "VulkanCommandManager.h"
#include "VulkanSwapchainManager.h"

#include "graphics/vulkan/pipeline/VulkanGraphicsPipeline.h"

#include "graphics/vulkan/resources/VulkanDescriptorManager.h"
#include "graphics/vulkan/resources/ubo/ObjectUniformBuffer.h"
#include "graphics/vulkan/resources/mesh/VulkanMeshManager.h"
#include "graphics/vulkan/resources/image/VulkanImageManager.h"

#include "graphics/vulkan/pipeline/VulkanFrameGraph.h"

class VulkanRenderer final : public GraphicsAPI, public VulkanEntityOwner<VulkanRenderer> {
public:
    VulkanRenderer() = default;
    ~VulkanRenderer() override;

    [[nodiscard]] bool init(Platform::Window& window) override;

    void shutdown() override;

    void drawFrame(const Camera& camera) override;

private:
    Platform::Window* _window = nullptr;

    VulkanContext           context;
    VulkanCommandManager    commandManager;
    VulkanSwapchainManager  swapchainManager;
    VulkanDescriptorManager descriptorManager;
    VulkanGraphicsPipeline  pipeline;

    ObjectUniformBuffer uniformBuffer;

    VulkanMeshManager meshManager;

    VulkanImageManager imageManager;

    VulkanImage depth;

    VulkanFrameGraph frameGraph;

    unsigned int currentFrame = 0;

    bool onFramebufferResize(std::string& errorMessage);

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
};

#endif //NOBLEENGINE_VULKANRENDERER_H
