#pragma once
#ifndef NOBLEENGINE_VULKANRENDERER_H
#define NOBLEENGINE_VULKANRENDERER_H

#include "graphics/GraphicsAPI.h"
#include "graphics/vulkan/common/VulkanEntityOwner.h"

#include "core/Platform.h"

#include "VulkanCommandManager.h"
#include "VulkanContext.h"
#include "VulkanSwapchainManager.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorManager.h"
#include "graphics/vulkan/resources/image/VulkanImageManager.h"
#include "graphics/vulkan/resources/mesh/VulkanMeshManager.h"
#include "graphics/vulkan/resources/ubo/FrameUniformBuffer.h"
#include "graphics/vulkan/resources/ubo/VulkanUniformBufferManager.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

#include "graphics/vulkan/pipeline/VulkanFrameGraph.h"
#include "graphics/vulkan/pipeline/VulkanPipelineManager.h"

class VulkanRenderer final : public GraphicsAPI, public VulkanEntityOwner<VulkanRenderer> {
public:
    VulkanRenderer() = default;
    ~VulkanRenderer() override;

    [[nodiscard]] bool init(Platform::Window& window, const std::vector<Object>& objects) override;

    void shutdown() override;

    void drawFrame(const Camera& camera) override;

private:
    Platform::Window* _window = nullptr;

    VulkanContext          context;
    VulkanSwapchainManager swapchainManager;
    VulkanCommandManager   commandManager;
    VulkanPipelineManager  pipelineManager;

    VulkanGraphicsPipeline pipelineComposite;
    VulkanGraphicsPipeline pipelineMeshRender;
    VulkanFrameGraph       frameGraph;

    VulkanDescriptorManager    descriptorManagerFrame;
    VulkanDescriptorManager    descriptorManagerObject;
    VulkanMeshManager          meshManager;
    VulkanImageManager         imageManager;
    VulkanUniformBufferManager uniformBufferManager;
    VulkanRenderObjectManager  renderObjectManager;

    FrameUniformBuffer frameUBO;
    std::unique_ptr<VulkanDescriptorSets> frameUBODescriptorSets;

    VulkanImage depth;
    VulkanImage compositeOutput;

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
