#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/rendergraph/VulkanRenderGraphBuilder.h"
#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"

#include "graphics/vulkan/resources/frame/VulkanFrameResources.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

struct VulkanDebugPassCreateContext {
    VulkanMeshManager&          meshManager;
    const VulkanFrameResources& frameResources;
    VulkanRenderObjectManager&  renderObjectManager;
    VulkanFrameCuller&          frameCuller;

    static VulkanDebugPassCreateContext build(const VulkanRenderGraphBuilderContext& context) {
        return {
            context.meshManager,
            context.frameResources,
            context.renderObjectManager,
            context.frameCuller
        };
    }
};

class VulkanDebugPass final : public VulkanRenderPass {
    using VulkanRenderPass::VulkanRenderPass;

public:
    [[nodiscard]] Expected<void> create(const VulkanDebugPassCreateContext& context);
};
