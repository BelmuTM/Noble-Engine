#include "MeshRenderPass.h"

#include "core/debug/ErrorHandling.h"

bool MeshRenderPass::create(
    const std::string&          path,
    VulkanShaderProgramManager& shaderProgramManager,
    VulkanPipelineManager&      pipelineManager,
    const VulkanFrameResources& frameResources,
    VulkanRenderObjectManager&  renderObjectManager,
    std::string&                errorMessage
) {
    const std::vector descriptorLayouts = {
        frameResources.getDescriptorManager().getLayout(), renderObjectManager.getDescriptorManager().getLayout()
    };

    VulkanShaderProgram program{};
    TRY(shaderProgramManager.load(program, path, false, errorMessage));

    VulkanGraphicsPipeline* pipeline = pipelineManager.allocatePipeline();
    TRY(pipelineManager.createGraphicsPipeline(pipeline, descriptorLayouts, objectDataGPUSize, program, errorMessage));

    const std::string& passName = std::filesystem::path(path).stem().string();

    setName(passName + "_MeshRenderPass");
    setPipeline(pipeline);
    setBindPoint(vk::PipelineBindPoint::eGraphics);
    setDepthAttachment(frameResources.getDepthBufferAttachment());

    for (const auto& renderObject : renderObjectManager.getRenderObjects()) {
        addObjectDrawCall(renderObject.get());
    }

    return true;
}
