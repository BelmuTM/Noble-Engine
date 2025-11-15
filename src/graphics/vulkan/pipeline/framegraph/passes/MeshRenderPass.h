#pragma once
#ifndef NOBLEENGINE_MESHRENDERPASS_H
#define NOBLEENGINE_MESHRENDERPASS_H

#include "graphics/vulkan/pipeline/framegraph/nodes/VulkanFramePass.h"

#include "graphics/vulkan/pipeline/VulkanPipelineManager.h"
#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"

#include "graphics/vulkan/resources/VulkanFrameResources.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

class MeshRenderPass final : public VulkanFramePass {
public:
    MeshRenderPass()  = default;
    ~MeshRenderPass() = default;

    MeshRenderPass(const MeshRenderPass&)            = delete;
    MeshRenderPass& operator=(const MeshRenderPass&) = delete;

    MeshRenderPass(MeshRenderPass&&)            = delete;
    MeshRenderPass& operator=(MeshRenderPass&&) = delete;

    [[nodiscard]] bool create(
        const std::string&          path,
        VulkanShaderProgramManager& shaderProgramManager,
        VulkanPipelineManager&      pipelineManager,
        const VulkanFrameResources& frameResources,
        VulkanRenderObjectManager&  renderObjectManager,
        std::string&                errorMessage
    );
};

#endif // NOBLEENGINE_MESHRENDERPASS_H
