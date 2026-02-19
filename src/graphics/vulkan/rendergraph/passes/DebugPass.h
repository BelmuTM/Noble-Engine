#pragma once

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
};

class DebugPass final : public VulkanRenderPass {
public:
     ~DebugPass() override;

    [[nodiscard]] bool create(
        const std::string&            path,
        const DebugPassCreateContext& context,
        std::string&                  errorMessage
    );

private:
    VulkanMeshManager _meshManager{};
};
