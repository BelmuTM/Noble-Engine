#pragma once
#ifndef NOBLEENGINE_COMPOSITEPASS_H
#define NOBLEENGINE_COMPOSITEPASS_H

#include "graphics/vulkan/pipeline/rendergraph/nodes/VulkanRenderPass.h"

#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"

#include "graphics/vulkan/resources/VulkanFrameResources.h"
#include "graphics/vulkan/resources/images/VulkanImageManager.h"

class CompositePass final : public VulkanRenderPass {
public:
    [[nodiscard]] bool create(
        const std::string&          path,
        VulkanShaderProgramManager& shaderProgramManager,
        const VulkanImageManager&   imageManager,
        const VulkanFrameResources& frameResources,
        std::string&                errorMessage
    );
};


#endif // NOBLEENGINE_COMPOSITEPASS_H
