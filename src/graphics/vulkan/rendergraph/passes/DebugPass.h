#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/rendergraph/VulkanRenderGraphBuilder.h"
#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"

#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"

#include "graphics/vulkan/resources/frame/VulkanFrameResources.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

struct DebugPassCreateContext {
    VulkanMeshManager&           meshManager;

    const VulkanFrameResources&  frameResources;
    const VulkanRenderResources& renderResources;

    VulkanRenderObjectManager&   renderObjectManager;

    VulkanFrameCuller&           frameCuller;

    VulkanShaderProgramManager&  shaderProgramManager;

    static DebugPassCreateContext build(const VulkanRenderGraphBuilderContext& context) {
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

class DebugPass final : public VulkanRenderPass {
    using VulkanRenderPass::VulkanRenderPass;

public:
    [[nodiscard]] Expected<void> create(const DebugPassCreateContext& context);
};
