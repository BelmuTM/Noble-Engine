#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/rendergraph/VulkanRenderGraph.h"
#include "graphics/vulkan/rendergraph/VulkanRenderPassFactory.h"

#include "graphics/vulkan/core/VulkanSwapchain.h"

#include "graphics/vulkan/pipeline/graphics/VulkanGraphicsPipelineManager.h"
#include "graphics/vulkan/pipeline/shaders/VulkanShaderProgramManager.h"

#include "graphics/vulkan/resources/frame/VulkanFrameResources.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

struct VulkanRenderGraphBuilderContext {
    VulkanRenderGraph&          renderGraph;

    const VulkanDevice&         device;
    const VulkanSwapchain&      swapchain;
    const VulkanCommandManager& commandManager;

    VulkanMeshManager&          meshManager;
    const VulkanImageManager&   imageManager;

    VulkanFrameResources&        frameResources;
    VulkanRenderResourceManager& renderResources;

    VulkanMaterialManager&      materialManager;
    VulkanRenderObjectManager&  renderObjectManager;

    VulkanFrameCuller&          frameCuller;

    VulkanShaderProgramManager&    shaderProgramManager;
    VulkanGraphicsPipelineManager& pipelineManager;
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

    [[nodiscard]] Expected<void> build();

    VulkanRenderGraphBuilder& registerResource(const VulkanRenderPassResourceDescriptor& descriptor) {
        _resourceDescriptors.push_back(descriptor);
        return *this;
    }

    VulkanRenderGraphBuilder& addPass(const VulkanRenderPassDescriptor& descriptor) {
        _passDescriptors.push_back(descriptor);
        return *this;
    }

private:
    [[nodiscard]] Expected<VulkanRenderPass*> allocatePass(const VulkanRenderPassDescriptor& descriptor) const;

    [[nodiscard]] Expected<void> allocateResources() const;

    [[nodiscard]] Expected<void> resolveAttachments(VulkanRenderPass* pass) const;

    [[nodiscard]] Expected<void> allocateDescriptors(VulkanRenderPass* pass) const;

    [[nodiscard]] Expected<void> resolveDescriptorLayouts(VulkanRenderPass* pass) const;

    [[nodiscard]] static Expected<void> resolvePushConstantRanges(VulkanRenderPass* pass);

    [[nodiscard]] Expected<void> createPipeline(VulkanRenderPass* pass) const;
    
    void scheduleResourceTransitions() const;

    const VulkanRenderGraphBuilderContext _context;

    const VulkanRenderPassFactory& _passFactory;

    std::vector<VulkanRenderPassResourceDescriptor> _resourceDescriptors{};

    std::vector<VulkanRenderPassDescriptor> _passDescriptors{};

    // Placeholder descriptor layout for passes with no incoming data
    vk::DescriptorSetLayout _emptyDescriptorLayout{};
};
