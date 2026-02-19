#pragma once

#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"

#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"

#include "graphics/vulkan/resources/VulkanFrameResources.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

struct DebugPassCreateContext {
    const VulkanFrameResources&  frameResources;
    const VulkanRenderResources& renderResources;
    VulkanRenderObjectManager&   renderObjectManager;
    VulkanShaderProgramManager&  shaderProgramManager;
};

class DebugPass final : public VulkanRenderPass {
public:
    [[nodiscard]] bool create(
        const std::string&            path,
        const DebugPassCreateContext& context,
        std::string&                  errorMessage
    );
};
