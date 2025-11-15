#pragma once
#ifndef NOBLEENGINE_COMPOSITEPASS_H
#define NOBLEENGINE_COMPOSITEPASS_H

#include "graphics/vulkan/pipeline/framegraph/nodes/VulkanFramePass.h"

#include "graphics/vulkan/pipeline/VulkanPipelineManager.h"
#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"
#include "graphics/vulkan/resources/VulkanFrameResources.h"

class CompositePass final : public VulkanFramePass {
public:
    CompositePass()  = default;
    ~CompositePass() = default;

    CompositePass(const CompositePass&)            = delete;
    CompositePass& operator=(const CompositePass&) = delete;

    CompositePass(CompositePass&&)            = delete;
    CompositePass& operator=(CompositePass&&) = delete;

    [[nodiscard]] bool create(
        const std::string&          path,
        VulkanShaderProgramManager& shaderProgramManager,
        VulkanPipelineManager&      pipelineManager,
        const VulkanFrameResources& frameResources,
        std::string&                errorMessage
    );
};


#endif // NOBLEENGINE_COMPOSITEPASS_H
