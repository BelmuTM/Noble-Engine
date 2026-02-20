#pragma once

#include "graphics/vulkan/rendergraph/VulkanRenderGraphBuilder.h"
#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"

#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"

#include "graphics/vulkan/resources/VulkanFrameResources.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

struct DebugPassCreateContext {
    const VulkanDevice&          device;
    const VulkanCommandManager&  commandManager;

    const VulkanFrameResources&  frameResources;
    const VulkanRenderResources& renderResources;
    VulkanRenderObjectManager&   renderObjectManager;
    VulkanShaderProgramManager&  shaderProgramManager;

    static DebugPassCreateContext build(const VulkanRenderGraphBuilderContext& context) {
        return {
            context.device,
            context.commandManager,
            context.frameResources,
            context.renderResources,
            context.renderObjectManager,
            context.shaderProgramManager
        };
    }
};

class DebugPass final : public VulkanRenderPass {
public:
    [[nodiscard]] bool create(
        const std::string&            path,
        const DebugPassCreateContext& context,
        std::string&                  errorMessage
    );

    void destroy() noexcept override;

private:
    VulkanMeshManager _meshManager{};
};
