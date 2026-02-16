#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/core/VulkanSwapchain.h"

#include "graphics/vulkan/pipeline/VulkanPipelineManager.h"
#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"
#include "graphics/vulkan/rendergraph/VulkanRenderGraph.h"

#include "graphics/vulkan/resources/VulkanFrameResources.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

struct VulkanRenderGraphBuilderContext {
    VulkanRenderGraph& renderGraph;

    VulkanMeshManager&          meshManager;
    const VulkanImageManager&   imageManager;
    VulkanFrameResources&       frameResources;
    VulkanRenderResources&      renderResources;
    VulkanRenderObjectManager&  renderObjectManager;
    VulkanShaderProgramManager& shaderProgramManager;
    VulkanPipelineManager&      pipelineManager;

    const VulkanSwapchain& swapchain;
};

class VulkanRenderGraphBuilder {
public:
    explicit VulkanRenderGraphBuilder(const VulkanRenderGraphBuilderContext& context) : _context(context) {}

    ~VulkanRenderGraphBuilder() = default;

    VulkanRenderGraphBuilder(const VulkanRenderGraphBuilder&)            = delete;
    VulkanRenderGraphBuilder& operator=(const VulkanRenderGraphBuilder&) = delete;

    VulkanRenderGraphBuilder(VulkanRenderGraphBuilder&&)            = delete;
    VulkanRenderGraphBuilder& operator=(VulkanRenderGraphBuilder&&) = delete;

    [[nodiscard]] bool build(std::string& errorMessage) const;

    [[nodiscard]] bool buildPasses(std::string& errorMessage) const;

private:
    const VulkanRenderGraphBuilderContext& _context;

    [[nodiscard]] bool createPass(const std::string& path, VulkanRenderPassType type, std::string& errorMessage) const;

    [[nodiscard]] bool attachSwapchainOutput(std::string& errorMessage) const;

    [[nodiscard]] bool createColorBuffers(std::string& errorMessage) const;

    [[nodiscard]] bool allocateDescriptors(std::string& errorMessage) const;

    [[nodiscard]] bool setupResourceTransitions(std::string& errorMessage) const;

    [[nodiscard]] bool createPipelines(std::string& errorMessage) const;
};
