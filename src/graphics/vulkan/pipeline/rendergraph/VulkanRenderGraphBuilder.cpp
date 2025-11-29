#include "VulkanRenderGraphBuilder.h"

#include "passes/CompositePass.h"
#include "passes/MeshRenderPass.h"

#include <ranges>

#include "core/debug/Logger.h"

bool VulkanRenderGraphBuilder::build(const VulkanRenderGraphBuilderContext& context, std::string& errorMessage) {
    TRY(buildPasses(
        context.renderGraph,
        context.meshManager,
        context.frameResources,
        context.renderResources,
        context.renderObjectManager,
        context.shaderProgramManager,
        errorMessage
    ));

    TRY(createColorAttachments(
        context.renderResources, context.imageManager, context.frameResources, context.renderGraph, errorMessage
    ));

    TRY(attachSwapchainOutput(context.swapchain, context.frameResources, context.renderGraph, errorMessage));

    TRY(allocateDescriptors(context.renderResources, context.renderGraph, errorMessage));

    TRY(setupResourceTransitions(context.renderResources, errorMessage));

    TRY(createPipelines(context.renderGraph, context.pipelineManager, errorMessage));

    context.shaderProgramManager.destroy();

    return true;
}

bool VulkanRenderGraphBuilder::buildPasses(
    VulkanRenderGraph&           renderGraph,
    VulkanMeshManager&           meshManager,
    const VulkanFrameResources&  frameResources,
    const VulkanRenderResources& renderResources,
    VulkanRenderObjectManager&   renderObjectManager,
    VulkanShaderProgramManager&  shaderProgramManager,
    std::string&                 errorMessage
) {
    auto meshRenderPass = std::make_unique<MeshRenderPass>();
    TRY(meshRenderPass->create(
        "mesh_render",
        frameResources,
        renderResources,
        renderObjectManager,
        shaderProgramManager,
        errorMessage
    ));

    renderGraph.addPass(std::move(meshRenderPass));

    auto compositePass0 = std::make_unique<CompositePass>();
    TRY(compositePass0->create(
        "composite_0",
        meshManager,
        frameResources,
        shaderProgramManager,
        errorMessage
    ));

    renderGraph.addPass(std::move(compositePass0));
    auto compositePass1 = std::make_unique<CompositePass>();
    TRY(compositePass1->create(
        "composite_1",
        meshManager,
        frameResources,
        shaderProgramManager,
        errorMessage
    ));

    renderGraph.addPass(std::move(compositePass1));

    return true;
}

bool VulkanRenderGraphBuilder::attachSwapchainOutput(
    const VulkanSwapchain& swapchain,
    VulkanFrameResources&  frameResources,
    VulkanRenderGraph&     renderGraph,
    std::string&           errorMessage
) {
    VulkanRenderPassResource swapchainOutput{};
    swapchainOutput
        .setName("Swapchain_Output")
        .setType(SwapchainOutput)
        .setImageResolver([&swapchain, &frameResources] {
            return swapchain.getImage(frameResources.getImageIndex());
        });

    VulkanRenderPassAttachment swapchainAttachment{};
    swapchainAttachment
        .setResource(swapchainOutput)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(defaultClearColor);

    VulkanRenderPass* lastPass = renderGraph.getPasses().back().get();

    if (lastPass->getColorAttachments().empty()) {
        errorMessage = "Failed to attach Vulkan swapchain output: last executing pass has no color attachments";
        return false;
    }

    // Attach the swapchain output to the first declared color attachment of the last executing pass
    lastPass->getColorAttachments().at(0) = std::make_unique<VulkanRenderPassAttachment>(swapchainAttachment);

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
    VulkanRenderResources& renderResources, VulkanRenderGraph& renderGraph, std::string& errorMessage
) {
    for (const auto& pass : renderGraph.getPasses()) {
        TRY(renderResources.allocateDescriptors(pass.get(), errorMessage));
    }

    return true;
}

bool VulkanRenderGraphBuilder::setupResourceTransitions(
    VulkanRenderResources& renderResources, std::string& errorMessage
) {
    for (const auto& [resourceName, writerPasses] : renderResources.getResourceWriters()) {
        auto it = renderResources.getResources().find(resourceName);
        if (it == renderResources.getResources().end()) continue;

        VulkanRenderPassResource* resource = it->second.get();

        for (VulkanRenderPass* writerPass : writerPasses) {
            if (!writerPass) continue;
            writerPass->addTransition({resource, vk::ImageLayout::eShaderReadOnlyOptimal});
        }
    }

    return true;
}

bool VulkanRenderGraphBuilder::createPipelines(
    VulkanRenderGraph& renderGraph, VulkanPipelineManager& pipelineManager, std::string& errorMessage
) {
    for (const auto& pass : renderGraph.getPasses()) {
        VulkanGraphicsPipeline* pipeline = pipelineManager.allocatePipeline();

        TRY(pipelineManager.createGraphicsPipeline(
            pipeline, pass->getPipelineDescriptor(), pass->getColorAttachments(), errorMessage
        ));

        pass->setPipeline(pipeline);
    }

    return true;
}
