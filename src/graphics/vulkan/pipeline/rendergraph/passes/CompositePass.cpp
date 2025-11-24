#include "CompositePass.h"

#include "core/debug/ErrorHandling.h"

bool CompositePass::create(
    const std::string&          path,
    VulkanMeshManager&          meshManager,
    const VulkanFrameResources& frameResources,
    VulkanShaderProgramManager& shaderProgramManager,
    std::string&                errorMessage
) {
    TRY(shaderProgramManager.load(getShaderProgram(), path, true, errorMessage));

    const std::string& passName = std::filesystem::path(path).stem().string();

    const VulkanPipelineDescriptor pipelineDescriptor{
        getShaderProgram(),
        {frameResources.getDescriptorManager().getLayout()}
    };

    setName(passName + "_CompositePass");
    setPipelineDescriptor(pipelineDescriptor);
    setBindPoint(vk::PipelineBindPoint::eGraphics);
    setDepthAttachment(frameResources.getDepthBufferAttachment());

    const VulkanMesh* fullscreenMesh = meshManager.allocateMesh(VulkanMesh::makeFullscreenTriangle());

    auto fullscreenDraw = std::make_unique<VulkanDrawCall>();
    fullscreenDraw->setMesh(fullscreenMesh);
    addDrawCall(std::move(fullscreenDraw));

    return true;
}
