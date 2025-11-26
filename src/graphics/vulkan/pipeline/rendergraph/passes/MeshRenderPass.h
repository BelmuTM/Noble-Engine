#pragma once
#ifndef NOBLEENGINE_MESHRENDERPASS_H
#define NOBLEENGINE_MESHRENDERPASS_H

#include "graphics/vulkan/pipeline/rendergraph/nodes/VulkanRenderPass.h"

#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"

#include "graphics/vulkan/resources/VulkanFrameResources.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

class MeshRenderPass final : public VulkanRenderPass {
public:
    [[nodiscard]] bool create(
        const std::string&           path,
        const VulkanFrameResources&  frameResources,
        const VulkanRenderResources& renderResources,
        VulkanRenderObjectManager&   renderObjectManager,
        VulkanShaderProgramManager&  shaderProgramManager,
        std::string&                 errorMessage
    );
};

#endif // NOBLEENGINE_MESHRENDERPASS_H
