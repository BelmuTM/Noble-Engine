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

    attachSwapchainOutput(context.swapchain, context.frameResources, context.renderGraph);

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

void VulkanRenderGraphBuilder::attachSwapchainOutput(
    const VulkanSwapchain& swapchain, VulkanFrameResources& frameResources, VulkanRenderGraph& renderGraph
) {
    VulkanRenderPassResource swapchainOutput{};
    swapchainOutput
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
    lastPass->addColorAttachmentAtIndex(0, swapchainAttachment);
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
    // Merge all descriptors globally
    for (const auto& pass : renderGraph.getPasses()) {
        for (const auto& [set, scheme] : pass->getShaderProgram()->getDescriptorSchemes()) {
            for (const auto& descriptor : scheme) {
                if (renderResources.getResources().contains(descriptor.name)) {
                    // Adding pass to this resource's readers
                    renderResources.addResourceReader(descriptor.name, pass.get());
                    // Adding descriptor to global scheme
                    renderResources.getDescriptorSchemes()[set].push_back(descriptor);
                }
            }
        }
    }

    // Allocate descriptor sets
    TRY(renderResources.allocateDescriptors(errorMessage));

    // Merge reflected layouts into each pass pipeline descriptor
    for (const auto& pass : renderGraph.getPasses()) {
        auto& descriptorLayouts = pass->getPipelineDescriptor().descriptorLayouts;

        for (const auto& manager : renderResources.getDescriptorManagers()) {
            vk::DescriptorSetLayout layout = manager->getLayout();

            if (std::ranges::find(descriptorLayouts, layout) == descriptorLayouts.end()) {
                descriptorLayouts.push_back(layout);
            }
        }
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
    for (const auto& renderPass : renderGraph.getPasses()) {
        VulkanGraphicsPipeline* pipeline = pipelineManager.allocatePipeline();

        TRY(pipelineManager.createGraphicsPipeline(
            pipeline, renderPass->getPipelineDescriptor(), renderPass->getColorAttachments(), errorMessage
        ));

        renderPass->setPipeline(pipeline);
    }

    return true;
}
