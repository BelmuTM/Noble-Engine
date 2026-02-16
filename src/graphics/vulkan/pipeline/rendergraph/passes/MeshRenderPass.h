#pragma once

#include "graphics/vulkan/pipeline/rendergraph/nodes/VulkanRenderPass.h"

#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"

#include "graphics/vulkan/resources/VulkanFrameResources.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

struct MeshRenderPassCreateContext {
    const VulkanFrameResources&  frameResources;
    const VulkanRenderResources& renderResources;
    VulkanRenderObjectManager&   renderObjectManager;
    VulkanShaderProgramManager&  shaderProgramManager;
};

class MeshRenderPass final : public VulkanRenderPass {
public:
    [[nodiscard]] bool create(
        const std::string&                 path,
        const MeshRenderPassCreateContext& context,
        std::string&                       errorMessage
    );
};
