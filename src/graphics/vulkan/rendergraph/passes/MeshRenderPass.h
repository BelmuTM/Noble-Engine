#pragma once

#include "graphics/vulkan/rendergraph/VulkanRenderGraphBuilder.h"
#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"

#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"

#include "graphics/vulkan/resources/VulkanFrameResources.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

struct MeshRenderPassCreateContext {
    const VulkanFrameResources&  frameResources;
    const VulkanRenderResources& renderResources;
    VulkanRenderObjectManager&   renderObjectManager;
    VulkanShaderProgramManager&  shaderProgramManager;

    static MeshRenderPassCreateContext build(const VulkanRenderGraphBuilderContext& context) {
        return {
            context.frameResources,
            context.renderResources,
            context.renderObjectManager,
            context.shaderProgramManager
        };
    }
};

class MeshRenderPass final : public VulkanRenderPass {
public:
    [[nodiscard]] bool create(
        const std::string&                 path,
        const MeshRenderPassCreateContext& context,
        std::string&                       errorMessage
    );
};
