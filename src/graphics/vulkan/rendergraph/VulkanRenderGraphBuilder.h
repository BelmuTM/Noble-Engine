#pragma once

#include "VulkanRenderPassFactory.h"

#include "graphics/vulkan/core/VulkanSwapchain.h"

#include "graphics/vulkan/pipeline/VulkanPipelineManager.h"
#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"
#include "graphics/vulkan/rendergraph/VulkanRenderGraph.h"

#include "graphics/vulkan/resources/VulkanFrameResources.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

struct VulkanRenderGraphBuilderContext {
    VulkanRenderGraph&          renderGraph;

    const VulkanDevice&         device;
    const VulkanSwapchain&      swapchain;
    const VulkanCommandManager& commandManager;

    VulkanMeshManager&          meshManager;
    const VulkanImageManager&   imageManager;
    VulkanFrameResources&       frameResources;
    VulkanRenderResources&      renderResources;
    VulkanRenderObjectManager&  renderObjectManager;
    VulkanShaderProgramManager& shaderProgramManager;
    VulkanPipelineManager&      pipelineManager;
};

class VulkanRenderGraphBuilder {
public:
    explicit VulkanRenderGraphBuilder(
        const VulkanRenderGraphBuilderContext& context, const VulkanRenderPassFactory& passFactory
    ) : _context(context), _passFactory(passFactory) {}

    ~VulkanRenderGraphBuilder() = default;

    VulkanRenderGraphBuilder(const VulkanRenderGraphBuilder&)            = delete;
    VulkanRenderGraphBuilder& operator=(const VulkanRenderGraphBuilder&) = delete;

    VulkanRenderGraphBuilder(VulkanRenderGraphBuilder&&)            = delete;
    VulkanRenderGraphBuilder& operator=(VulkanRenderGraphBuilder&&) = delete;

    [[nodiscard]] bool build(
        const std::vector<VulkanRenderPassDescriptor>& passDescriptors, std::string& errorMessage
    ) const;

private:
    [[nodiscard]] bool createPasses(
        const std::vector<VulkanRenderPassDescriptor>& passDescriptors, std::string& errorMessage
    ) const;

    [[nodiscard]] bool attachSwapchainOutput(std::string& errorMessage) const;

    [[nodiscard]] bool createColorBuffers(std::string& errorMessage) const;

    [[nodiscard]] bool allocateDescriptors(std::string& errorMessage) const;

    [[nodiscard]] bool createPipelines(std::string& errorMessage) const;

    const VulkanRenderGraphBuilderContext& _context;

    const VulkanRenderPassFactory& _passFactory;
};
