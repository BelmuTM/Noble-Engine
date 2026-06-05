#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/rendergraph/VulkanRenderGraphBuilder.h"
#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"

#include "graphics/vulkan/resources/frame/VulkanFrameResources.h"
#include "graphics/vulkan/resources/meshes/VulkanMeshManager.h"

struct VulkanCompositePassCreateContext {
    VulkanMeshManager&          meshManager;
    const VulkanFrameResources& frameResources;

    static VulkanCompositePassCreateContext build(const VulkanRenderGraphBuilderContext& context) {
        return {
            context.meshManager,
            context.frameResources
        };
    }
};

class VulkanCompositePass final : public VulkanRenderPass {
    using VulkanRenderPass::VulkanRenderPass;

public:
    [[nodiscard]] Expected<void> create(const VulkanCompositePassCreateContext& context);
};
