#include "VulkanRenderGraphBuilder.h"

#include "graphics/vulkan/pipeline/rendergraph/passes/MeshRenderPass.h"

bool VulkanRenderGraphBuilder::buildPasses(
    VulkanRenderGraph&          renderGraph,
    VulkanShaderProgramManager& shaderProgramManager,
    const VulkanImageManager&   imageManager,
    VulkanFrameResources&       frameResources,
    VulkanRenderObjectManager&  renderObjectManager,
    std::string&                errorMessage
) {

    auto meshRenderPass = std::make_unique<MeshRenderPass>();
    TRY(meshRenderPass->create(
        "mesh_render",
        shaderProgramManager,
        imageManager,
        frameResources,
        renderObjectManager,
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
