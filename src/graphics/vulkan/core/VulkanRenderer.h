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
#include "graphics/vulkan/resources/images/VulkanImageManager.h"
#include "graphics/vulkan/resources/meshes/VulkanMeshManager.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"
#include "graphics/vulkan/resources/ubo/VulkanUniformBufferManager.h"
#include "graphics/vulkan/resources/VulkanFrameResources.h"

#include "graphics/vulkan/pipeline/VulkanPipelineManager.h"
#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"
#include "graphics/vulkan/pipeline/rendergraph/VulkanRenderGraph.h"

class VulkanRenderer final : public GraphicsAPI, public VulkanEntityOwner<VulkanRenderer> {
public:
    explicit VulkanRenderer(uint32_t framesInFlight = 2);

    ~VulkanRenderer() override;

    [[nodiscard]] bool init(
        Platform::Window& window, const ObjectsVector& objects, std::string& errorMessage
    ) override;

    void shutdown() override;

    void drawFrame(const Camera& camera) override;

private:
    Platform::Window* _window = nullptr;

    uint32_t _framesInFlight;

    unsigned int currentFrame = 0;

    VulkanContext context{};

    VulkanSwapchainManager     swapchainManager{};
    VulkanCommandManager       commandManager{};
    VulkanMeshManager          meshManager{};
    VulkanImageManager         imageManager{};
    VulkanUniformBufferManager uniformBufferManager{};

    VulkanFrameResources      frameResources{};
    VulkanRenderObjectManager renderObjectManager{};

    VulkanShaderProgramManager shaderProgramManager{};
    VulkanPipelineManager      pipelineManager{};
    VulkanRenderGraph          renderGraph{};

    [[nodiscard]] bool onFramebufferResize(std::string& errorMessage);

    [[nodiscard]] bool recordCommandBuffer(
        vk::CommandBuffer commandBuffer, uint32_t imageIndex, std::string& errorMessage
    );

    [[nodiscard]] bool recordCurrentCommandBuffer(uint32_t imageIndex, std::string& errorMessage);

    [[nodiscard]] bool submitCurrentCommandBuffer(uint32_t imageIndex, std::string& errorMessage, bool& discardLogging);
};

#endif //NOBLEENGINE_VULKANRENDERER_H
