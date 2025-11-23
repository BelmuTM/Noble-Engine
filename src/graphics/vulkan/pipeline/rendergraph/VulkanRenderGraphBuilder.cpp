#include "VulkanRenderGraphBuilder.h"

#include <ranges>

#include "passes/CompositePass.h"
#include "passes/MeshRenderPass.h"

bool VulkanRenderGraphBuilder::buildPasses(
    VulkanRenderGraph&          renderGraph,
    VulkanMeshManager&          meshManager,
    const VulkanImageManager&   imageManager,
    VulkanFrameResources&       frameResources,
    VulkanRenderResources&      renderResources,
    VulkanRenderObjectManager&  renderObjectManager,
    VulkanShaderProgramManager& shaderProgramManager,
    std::string&                errorMessage
) {
    auto meshRenderPass = std::make_unique<MeshRenderPass>();
    TRY(meshRenderPass->create(
        "mesh_render",
        imageManager,
        frameResources,
        renderResources,
        renderObjectManager,
        shaderProgramManager,
        errorMessage
    ));

    renderGraph.addPass(std::move(meshRenderPass));

    auto compositePass = std::make_unique<CompositePass>();
    TRY(compositePass->create(
        "composite",
        meshManager,
        imageManager,
        frameResources,
        renderResources,
        shaderProgramManager,
        errorMessage
    ));

    renderGraph.addPass(std::move(compositePass));

    for (const auto& pass : renderGraph.getPasses()) {
        pass->_sampledInputs.clear();

        std::unordered_set<std::string> outputs;
        for (const auto& attachment : pass->getColorAttachments()) {
            outputs.insert(attachment->resource.name);
        }

        const auto& shaderProgram = pass->getPipelineDescriptor().shaderProgram;
        for (const auto& scheme : shaderProgram->getDescriptorSchemes() | std::views::values) {
            for (const auto& bindingInfo : scheme) {
                if (bindingInfo.type == vk::DescriptorType::eCombinedImageSampler ||
                    bindingInfo.type == vk::DescriptorType::eSampledImage) {

                    auto it = renderResources.getResources().find(bindingInfo.name);
                    if (it != renderResources.getResources().end()) {
                        VulkanRenderPassResource* resource = it->second.get();
                        if (!outputs.contains(resource->name)) {
                            pass->_sampledInputs.push_back(resource);
                        }
                    }
                }
            }
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
