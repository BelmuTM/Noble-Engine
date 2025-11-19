#include "VulkanRenderGraphBuilder.h"

#include "passes/MeshRenderPass.h"
#include "passes/CompositePass.h"

bool VulkanRenderGraphBuilder::buildPasses(
    VulkanRenderGraph&          renderGraph,
    VulkanMeshManager&          meshManager,
    const VulkanImageManager&   imageManager,
    VulkanFrameResources&       frameResources,
    VulkanRenderObjectManager&  renderObjectManager,
    VulkanShaderProgramManager& shaderProgramManager,
    std::string&                errorMessage
) {
    auto compositePass = std::make_unique<CompositePass>();
    TRY(compositePass->create(
        "composite",
        meshManager,
        imageManager,
        frameResources,
        shaderProgramManager,
        errorMessage
    ));

    renderGraph.addPass(std::move(compositePass));

    auto meshRenderPass = std::make_unique<MeshRenderPass>();
    TRY(meshRenderPass->create(
        "mesh_render",
        imageManager,
        frameResources,
        renderObjectManager,
        shaderProgramManager,
        errorMessage
    ));

    renderGraph.addPass(std::move(meshRenderPass));

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
