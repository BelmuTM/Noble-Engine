#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/rendergraph/VulkanRenderGraphBuilder.h"
#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"

#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"

#include "graphics/vulkan/resources/frame/VulkanFrameResources.h"
#include "graphics/vulkan/resources/meshes/VulkanMeshManager.h"

struct VulkanCompositePassCreateContext {
    VulkanMeshManager&          meshManager;
    const VulkanFrameResources& frameResources;

    VulkanShaderProgramManager& shaderProgramManager;

    static VulkanCompositePassCreateContext build(const VulkanRenderGraphBuilderContext& context) {
        return {
            context.meshManager,
            context.frameResources,
            context.shaderProgramManager
        };
    }
};

class VulkanCompositePass final : public VulkanRenderPass {
    using VulkanRenderPass::VulkanRenderPass;

public:
    [[nodiscard]] Expected<void> create(const VulkanCompositePassCreateContext& context);
};
