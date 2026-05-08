#pragma once

#include "core/debug/ErrorHandling.h"

#include "VulkanRenderPassFactory.h"

#include "graphics/vulkan/core/VulkanSwapchain.h"

#include "graphics/vulkan/pipeline/VulkanPipelineManager.h"
#include "graphics/vulkan/pipeline/VulkanShaderProgramManager.h"
#include "graphics/vulkan/rendergraph/VulkanRenderGraph.h"

#include "graphics/vulkan/resources/frame/VulkanFrameResources.h"
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

    VulkanMaterialManager&      materialManager;
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

    [[nodiscard]] Expected<void> build(const std::vector<VulkanRenderPassDescriptor>& passDescriptors) const;

private:
    [[nodiscard]] Expected<void> createPasses(const std::vector<VulkanRenderPassDescriptor>& passDescriptors) const;

    [[nodiscard]] Expected<void> attachSwapchainOutput() const;

    [[nodiscard]] Expected<void> createColorBuffers() const;

    [[nodiscard]] Expected<void> allocateDescriptors() const;

    [[nodiscard]] Expected<void> createPipelines() const;

    void scheduleDepthLoadOps() const;
    
    void scheduleResourceTransitions() const;

    const VulkanRenderGraphBuilderContext& _context;

    const VulkanRenderPassFactory& _passFactory;
};
