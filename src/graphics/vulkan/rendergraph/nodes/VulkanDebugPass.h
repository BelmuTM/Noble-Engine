#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/rendergraph/VulkanRenderGraphBuilder.h"
#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"

#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"

#include "graphics/vulkan/resources/frame/VulkanFrameResources.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

struct VulkanDebugPassCreateContext {
    VulkanMeshManager&                 meshManager;

    const VulkanFrameResources&        frameResources;
    const VulkanRenderResourceManager& renderResources;

    VulkanRenderObjectManager&         renderObjectManager;

    VulkanFrameCuller&                 frameCuller;

    VulkanShaderProgramManager&        shaderProgramManager;

    static VulkanDebugPassCreateContext build(const VulkanRenderGraphBuilderContext& context) {
        return {
            context.meshManager,
            context.frameResources,
            context.renderResources,
            context.renderObjectManager,
            context.frameCuller,
            context.shaderProgramManager
        };
    }
};

class VulkanDebugPass final : public VulkanRenderPass {
    using VulkanRenderPass::VulkanRenderPass;

public:
    [[nodiscard]] Expected<void> create(const VulkanDebugPassCreateContext& context);
};
