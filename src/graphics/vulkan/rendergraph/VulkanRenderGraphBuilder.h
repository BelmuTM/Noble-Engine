#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/pipeline/VulkanPipelineManager.h"

#include "graphics/vulkan/rendergraph/VulkanRenderGraph.h"
#include "graphics/vulkan/rendergraph/draw/VulkanDrawBatchBuilder.h"

#include "graphics/vulkan/resources/frame/VulkanFrameResources.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

struct VulkanRenderGraphBuilderContext {
    VulkanRenderGraph&          renderGraph;

    const VulkanDevice&         device;

    VulkanFrameResources&       frameResources;
    VulkanGraphResourceManager& graphResources;

    VulkanMaterialManager&      materialManager;
    VulkanRenderObjectManager&  renderObjectManager;

    VulkanDrawBatchBuilder&     drawBatchBuilder;

    VulkanPipelineManager&      pipelineManager;
};

class VulkanRenderGraphBuilder {
public:
    explicit VulkanRenderGraphBuilder(const VulkanRenderGraphBuilderContext& context) : _context(context) {}

    ~VulkanRenderGraphBuilder() = default;

    VulkanRenderGraphBuilder(const VulkanRenderGraphBuilder&)            = delete;
    VulkanRenderGraphBuilder& operator=(const VulkanRenderGraphBuilder&) = delete;

    VulkanRenderGraphBuilder(VulkanRenderGraphBuilder&&)            = delete;
    VulkanRenderGraphBuilder& operator=(VulkanRenderGraphBuilder&&) = delete;

    [[nodiscard]] Expected<void> build();

    VulkanRenderGraphBuilder& addPass(const VulkanPassDescriptor& descriptor) {
        _passDescriptors.push_back(descriptor);
        return *this;
    }

private:
    using ResourceStates = std::unordered_map<VulkanGraphResource*, VulkanGraphResourceState>;

    [[nodiscard]] Expected<VulkanPass*> allocatePass(const VulkanPassDescriptor& descriptor) const;

    [[nodiscard]] Expected<void> scheduleTransitions(
        VulkanPass* pass, ResourceStates& resourceStates, vk::PipelineStageFlags2 shaderStages
    ) const;

    [[nodiscard]] Expected<void> resolveDescriptorLayouts(
        VulkanPipelineLayoutDescriptor&    pipelineLayoutDescriptor,
        const VulkanPassDescriptorBinding* passDescriptorBinding
    ) const;

    Expected<void> linkPipelines(
        VulkanPass* pass, const VulkanPassDescriptorBinding* binding
    ) const;

    const VulkanRenderGraphBuilderContext _context;

    std::vector<VulkanPassDescriptor> _passDescriptors{};

    // Placeholder descriptor layout for passes with no incoming data
    vk::DescriptorSetLayout _emptyDescriptorLayout{};
};
