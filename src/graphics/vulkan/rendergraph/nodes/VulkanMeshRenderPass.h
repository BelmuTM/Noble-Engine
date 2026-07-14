#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/rendergraph/VulkanRenderGraphBuilder.h"
#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"

#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

struct VulkanMeshRenderPassCreateContext {
    VulkanRenderObjectManager& renderObjectManager;

    static VulkanMeshRenderPassCreateContext build(const VulkanRenderGraphBuilderContext& context) {
        return {
            context.renderObjectManager,
        };
    }
};

class VulkanMeshRenderPass final : public VulkanRenderPass {
    using VulkanRenderPass::VulkanRenderPass;

public:
    [[nodiscard]] Expected<void> create(const VulkanMeshRenderPassCreateContext& context);
};
