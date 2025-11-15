#include "CompositePass.h"

#include "core/debug/ErrorHandling.h"

bool CompositePass::create(
    const std::string&          path,
    VulkanShaderProgramManager& shaderProgramManager,
    VulkanPipelineManager&      pipelineManager,
    const VulkanFrameResources& frameResources,
    std::string&                errorMessage
) {
    const std::vector descriptorLayouts = {frameResources.getDescriptorManager().getLayout()};

    VulkanShaderProgram program{};
    TRY(shaderProgramManager.load(program, path, false, errorMessage));

    VulkanGraphicsPipeline* pipeline = pipelineManager.allocatePipeline();
    TRY(pipelineManager.createGraphicsPipeline(pipeline, descriptorLayouts, objectDataGPUSize, program, errorMessage));

    const std::string& passName = std::filesystem::path(path).stem().string();

    auto fullScreenDraw = VulkanMesh::makeFullscreenTriangle();

    setName(passName + "_CompositePass");
    setPipeline(pipeline);
    setBindPoint(vk::PipelineBindPoint::eGraphics);
    setDepthAttachment(frameResources.getDepthBufferAttachment());
    //addColorAttachment();

    auto fullscreenDraw = std::make_unique<VulkanDrawCall>();
    fullscreenDraw->setMesh(VulkanMesh::makeFullscreenTriangle());
    addDrawCall(std::move(fullscreenDraw));

    return true;
}
