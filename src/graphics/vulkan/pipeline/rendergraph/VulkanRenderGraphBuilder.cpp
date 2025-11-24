#include "VulkanRenderGraphBuilder.h"

#include "passes/CompositePass.h"
#include "passes/MeshRenderPass.h"

#include <ranges>

bool VulkanRenderGraphBuilder::build(const VulkanRenderGraphBuilderContext& context, std::string& errorMessage) {
    TRY(buildPasses(
        context.renderGraph,
        context.meshManager,
        context.frameResources,
        context.renderObjectManager,
        context.shaderProgramManager,
        errorMessage
    ));

    context.renderGraph.attachSwapchainOutput(context.swapchain);

    TRY(createColorAttachments(
        context.renderResources, context.imageManager, context.frameResources, context.renderGraph, errorMessage
    ));

    TRY(allocateDescriptors(context.renderResources, context.renderGraph, errorMessage));

    TRY(setupResourceTransitions(context.renderResources, errorMessage));

    TRY(createPipelines(context.renderGraph, context.pipelineManager, errorMessage));

    context.shaderProgramManager.destroy();

    return true;
}

bool VulkanRenderGraphBuilder::buildPasses(
    VulkanRenderGraph&          renderGraph,
    VulkanMeshManager&          meshManager,
    const VulkanFrameResources& frameResources,
    VulkanRenderObjectManager&  renderObjectManager,
    VulkanShaderProgramManager& shaderProgramManager,
    std::string&                errorMessage
) {
    auto meshRenderPass = std::make_unique<MeshRenderPass>();
    TRY(meshRenderPass->create(
        "mesh_render",
        frameResources,
        renderObjectManager,
        shaderProgramManager,
        errorMessage
    ));

    renderGraph.addPass(std::move(meshRenderPass));

    auto compositePass = std::make_unique<CompositePass>();
    TRY(compositePass->create(
        "composite",
        meshManager,
        frameResources,
        shaderProgramManager,
        errorMessage
    ));

    renderGraph.addPass(std::move(compositePass));

    return true;
}

bool VulkanRenderGraphBuilder::createColorAttachments(
    VulkanRenderResources&    renderResources,
    const VulkanImageManager& imageManager,
    VulkanFrameResources&     frameResources,
    VulkanRenderGraph&        renderGraph,
    std::string&              errorMessage
) {
    for (auto& pass : renderGraph.getPasses()) {
        TRY(renderResources.createColorAttachments(pass.get(), imageManager, frameResources, errorMessage));
    }

    return true;
}

bool VulkanRenderGraphBuilder::allocateDescriptors(
    VulkanRenderResources& renderResources,
    VulkanRenderGraph&     renderGraph,
    std::string&           errorMessage
) {
    for (auto& pass : renderGraph.getPasses()) {
        TRY(renderResources.allocateDescriptors(pass.get(), errorMessage));
    }

    return true;
}

bool VulkanRenderGraphBuilder::setupResourceTransitions(
    const VulkanRenderResources& renderResources, std::string& errorMessage
) {
    for (const auto& resourceName : renderResources.getResourceReaders() | std::views::keys) {
        auto it = renderResources.getResources().find(resourceName);
        if (it == renderResources.getResources().end()) continue;

        VulkanRenderPassResource* resource = it->second.get();

        if (!renderResources.getResourceWriters().contains(resourceName)) continue;

        for (VulkanRenderPass* writerPass : renderResources.getResourceWriters().at(resourceName)) {
            if (!writerPass) continue;
            writerPass->addTransition({resource, vk::ImageLayout::eShaderReadOnlyOptimal});
        }
    }

    return true;
}

bool VulkanRenderGraphBuilder::createPipelines(
    VulkanRenderGraph& renderGraph, VulkanPipelineManager& pipelineManager, std::string& errorMessage
) {
    for (const auto& renderPass : renderGraph.getPasses()) {
        VulkanGraphicsPipeline* pipeline = pipelineManager.allocatePipeline();

        TRY(pipelineManager.createGraphicsPipeline(
            pipeline, renderPass->getPipelineDescriptor(), renderPass->getColorAttachments(), errorMessage
        ));

        renderPass->setPipeline(pipeline);
    }

    return true;
}
