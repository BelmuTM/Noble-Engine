#pragma once

#include "graphics/GraphicsAPI.h"
#include "graphics/vulkan/common/VulkanEntityOwner.h"

#include "graphics/vulkan/core/VulkanCommandManager.h"
#include "graphics/vulkan/core/VulkanContext.h"
#include "graphics/vulkan/core/VulkanSwapchainManager.h"

#include "graphics/vulkan/resources/frame/VulkanFrameResources.h"
#include "graphics/vulkan/resources/images/VulkanImageManager.h"
#include "graphics/vulkan/resources/meshes/VulkanMeshManager.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

#include "graphics/vulkan/resources/ssbo/VulkanStorageBufferManager.h"
#include "graphics/vulkan/resources/ubo/VulkanUniformBufferManager.h"

#include "graphics/vulkan/pipeline/VulkanPipelineManager.h"
#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"
#include "graphics/vulkan/rendergraph/VulkanRenderGraph.h"

class VulkanRenderer : public GraphicsAPI, public VulkanEntityOwner<VulkanRenderer> {
public:
    explicit VulkanRenderer(std::uint32_t framesInFlight = 2);

    [[nodiscard]] bool init(
        Window&              window,
        const AssetManager&  assetManager,
        const ObjectManager& objectManager,
        std::string&         errorMessage
    ) override;

    void shutdown() override;

    void drawFrame(const FrameUniforms& uniforms) override;

    std::uint32_t primitiveCount = 0;

private:
    [[nodiscard]] bool onFramebufferResize(std::string& errorMessage);

    [[nodiscard]] bool recordCommandBuffer(
        vk::CommandBuffer commandBuffer, std::uint32_t imageIndex, std::string& errorMessage
    );

    [[nodiscard]] bool recordCurrentCommandBuffer(std::uint32_t imageIndex, std::string& errorMessage);

    [[nodiscard]] bool submitCurrentCommandBuffer(std::uint32_t imageIndex, std::string& errorMessage, bool& discardLogging);

    Window* _window = nullptr;

    std::uint32_t _framesInFlight = 0;

    unsigned int currentFrame = 0;

    VulkanContext context{};

    VulkanSwapchainManager     swapchainManager{};
    VulkanCommandManager       commandManager{};
    VulkanMeshManager          meshManager{};
    VulkanImageManager         imageManager{};

    VulkanStorageBufferManager storageBufferManager{};
    VulkanUniformBufferManager uniformBufferManager{};

    VulkanFrameResources      frameResources{};
    VulkanRenderResources     renderResources{};

    VulkanMaterialManager     materialManager{};
    VulkanRenderObjectManager renderObjectManager{};

    VulkanFrameDraws          frameDraws{};

    VulkanShaderProgramManager shaderProgramManager{};
    VulkanPipelineManager      pipelineManager{};
    VulkanRenderGraph          renderGraph{};
};
