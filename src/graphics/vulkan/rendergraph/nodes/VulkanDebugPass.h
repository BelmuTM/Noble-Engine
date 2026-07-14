#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/rendergraph/VulkanRenderGraphBuilder.h"
#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"

#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

struct VulkanDebugPassCreateContext {
    VulkanMeshManager&         meshManager;
    VulkanRenderObjectManager& renderObjectManager;

    static VulkanDebugPassCreateContext build(const VulkanRenderGraphBuilderContext& context) {
        return {
            context.meshManager,
            context.renderObjectManager
        };
    }
};

class VulkanDebugPass final : public VulkanRenderPass {
    using VulkanRenderPass::VulkanRenderPass;

public:
    [[nodiscard]] Expected<void> create(const VulkanDebugPassCreateContext& context);
};
