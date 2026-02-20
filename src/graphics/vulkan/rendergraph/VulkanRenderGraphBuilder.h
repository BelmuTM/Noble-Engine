#pragma once

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
    explicit VulkanRenderGraphBuilder(const VulkanRenderGraphBuilderContext& context) : _context(context) {}

    ~VulkanRenderGraphBuilder() = default;

    VulkanRenderGraphBuilder(const VulkanRenderGraphBuilder&)            = delete;
    VulkanRenderGraphBuilder& operator=(const VulkanRenderGraphBuilder&) = delete;

    VulkanRenderGraphBuilder(VulkanRenderGraphBuilder&&)            = delete;
    VulkanRenderGraphBuilder& operator=(VulkanRenderGraphBuilder&&) = delete;

    [[nodiscard]] bool build(std::string& errorMessage);

    [[nodiscard]] bool buildPasses(std::string& errorMessage) const;

private:
    template <typename PassType, typename PassCreateContext>
    [[nodiscard]] static std::unique_ptr<VulkanRenderPass> createPassFactory(
        const std::string& path, const VulkanRenderGraphBuilderContext& context, std::string& errorMessage
    ) {
        auto pass = std::make_unique<PassType>();

        if (!pass->create(path, PassCreateContext::build(context), errorMessage)) {
            return nullptr;
        }

        return pass;
    }

    [[nodiscard]] bool createPass(const std::string& path, VulkanRenderPassType type, std::string& errorMessage) const;

    [[nodiscard]] bool attachSwapchainOutput(std::string& errorMessage) const;

    [[nodiscard]] bool createColorBuffers(std::string& errorMessage) const;

    [[nodiscard]] bool allocateDescriptors(std::string& errorMessage) const;

    [[nodiscard]] bool setupResourceTransitions(std::string& errorMessage) const;

    [[nodiscard]] bool createPipelines(std::string& errorMessage) const;

    const VulkanRenderGraphBuilderContext& _context;

    using PassFactoryFunction = std::unique_ptr<VulkanRenderPass>(
        const std::string&, const VulkanRenderGraphBuilderContext&, std::string&
    );

    std::unordered_map<VulkanRenderPassType, PassFactoryFunction*> _factories{};
};
