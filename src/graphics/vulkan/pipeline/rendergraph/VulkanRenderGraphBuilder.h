#pragma once
#ifndef NOBLEENGINE_VULKANRENDERGRAPHBUILDER_H
#define NOBLEENGINE_VULKANRENDERGRAPHBUILDER_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/pipeline/rendergraph/VulkanRenderGraph.h"

#include "graphics/vulkan/pipeline/VulkanPipelineManager.h"
#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"

#include "graphics/vulkan/resources/VulkanFrameResources.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

class VulkanRenderGraphBuilder {
public:
    VulkanRenderGraphBuilder()  = default;
    ~VulkanRenderGraphBuilder() = default;

    VulkanRenderGraphBuilder(const VulkanRenderGraphBuilder&)            = delete;
    VulkanRenderGraphBuilder& operator=(const VulkanRenderGraphBuilder&) = delete;

    VulkanRenderGraphBuilder(VulkanRenderGraphBuilder&&)            = delete;
    VulkanRenderGraphBuilder& operator=(VulkanRenderGraphBuilder&&) = delete;

    [[nodiscard]] static bool buildPasses(
        VulkanRenderGraph&          renderGraph,
        VulkanMeshManager&          meshManager,
        const VulkanImageManager&   imageManager,
        VulkanFrameResources&       frameResources,
        VulkanRenderObjectManager&  renderObjectManager,
        VulkanShaderProgramManager& shaderProgramManager,
        std::string&                errorMessage
    );

    [[nodiscard]] static bool createPipelines(
        VulkanRenderGraph& renderGraph, VulkanPipelineManager& pipelineManager, std::string& errorMessage
    );
};

#endif // NOBLEENGINE_VULKANRENDERGRAPHBUILDER_H
