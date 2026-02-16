#pragma once

#include "graphics/vulkan/pipeline/rendergraph/nodes/VulkanRenderPass.h"

#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"

#include "graphics/vulkan/resources/VulkanFrameResources.h"
#include "graphics/vulkan/resources/meshes/VulkanMeshManager.h"

struct CompositePassCreateContext {
    VulkanMeshManager&          meshManager;
    const VulkanFrameResources& frameResources;
    VulkanShaderProgramManager& shaderProgramManager;
};

class CompositePass final : public VulkanRenderPass {
public:
    [[nodiscard]] bool create(
        const std::string&                path,
        const CompositePassCreateContext& context,
        std::string&                      errorMessage
    );
};
