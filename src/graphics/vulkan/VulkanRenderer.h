#pragma once

#include "core/debug/ErrorHandling.h"

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

#include "graphics/vulkan/pipeline/VulkanGraphicsPipelineManager.h"
#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"
#include "graphics/vulkan/rendergraph/VulkanRenderGraph.h"

class VulkanRenderer final : public GraphicsAPI, public VulkanEntityOwner<VulkanRenderer> {
public:
    explicit VulkanRenderer(std::uint32_t framesInFlight = 2);

    [[nodiscard]] Expected<void> init(
        Window&              window,
        const AssetManager&  assetManager,
        const ObjectManager& objectManager
    ) override;

    void shutdown() override;

    Expected<void> drawFrame(const FrameUniforms& uniforms) override;

    std::uint64_t primitiveCount = 0;

private:
    [[nodiscard]] Expected<void> onFramebufferResize();

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

    VulkanFrameResources        frameResources{};
    VulkanRenderResourceManager renderResources{};

    VulkanMaterialManager      materialManager{};
    VulkanRenderObjectManager  renderObjectManager{};

    VulkanFrameCuller          frameCuller{};

    VulkanShaderProgramManager    shaderProgramManager{};
    VulkanGraphicsPipelineManager pipelineManager{};
    VulkanRenderGraph             renderGraph{};
};
