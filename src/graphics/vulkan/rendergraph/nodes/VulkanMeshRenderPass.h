#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/rendergraph/VulkanRenderGraphBuilder.h"
#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"

#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"

#include "graphics/vulkan/resources/frame/VulkanFrameResources.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

#include "graphics/vulkan/rendergraph/draw/VulkanFrameCuller.h"

struct VulkanMeshRenderPassCreateContext {
    const VulkanFrameResources&        frameResources;
    const VulkanRenderResourceManager& renderResources;

    VulkanMaterialManager&             materialManager;
    VulkanRenderObjectManager&         renderObjectManager;
    VulkanFrameCuller&                 frameCuller;

    VulkanShaderProgramManager&        shaderProgramManager;

    static VulkanMeshRenderPassCreateContext build(const VulkanRenderGraphBuilderContext& context) {
        return {
            context.frameResources,
            context.renderResources,
            context.materialManager,
            context.renderObjectManager,
            context.frameCuller,
            context.shaderProgramManager
        };
    }
};

class VulkanMeshRenderPass final : public VulkanRenderPass {
    using VulkanRenderPass::VulkanRenderPass;

public:
    [[nodiscard]] Expected<void> create(const VulkanMeshRenderPassCreateContext& context);
};
