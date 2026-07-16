#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/rendergraph/VulkanRenderGraphBuilder.h"
#include "graphics/vulkan/rendergraph/nodes/VulkanGraphicsPass.h"

#include "graphics/vulkan/resources/meshes/VulkanMeshManager.h"

struct VulkanCompositePassCreateContext {
    VulkanMeshManager& meshManager;

    static VulkanCompositePassCreateContext build(const VulkanRenderGraphBuilderContext& context) {
        return {
            context.meshManager
        };
    }
};

class VulkanCompositePass final : public VulkanGraphicsPass {
    using VulkanGraphicsPass::VulkanGraphicsPass;

public:
    [[nodiscard]] Expected<void> create(const VulkanCompositePassCreateContext& context);
};
