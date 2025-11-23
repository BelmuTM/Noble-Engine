#include "CompositePass.h"

#include "core/debug/ErrorHandling.h"

bool CompositePass::create(
    const std::string&          path,
    VulkanMeshManager&          meshManager,
    const VulkanImageManager&   imageManager,
    VulkanFrameResources&       frameResources,
    VulkanRenderResources&      renderResources,
    VulkanShaderProgramManager& shaderProgramManager,
    std::string&                errorMessage
) {
    VulkanShaderProgram* program;
    TRY(shaderProgramManager.load(program, path, true, errorMessage));

    TRY(createColorAttachments(*program, imageManager, frameResources, renderResources, errorMessage));

    VulkanPipelineDescriptor pipelineDescriptor{
        program,
        {frameResources.getDescriptorManager().getLayout()}
    };

    TRY(renderResources.allocateDescriptors(pipelineDescriptor, program->getDescriptorSchemes(), errorMessage));

    setPipelineDescriptor(pipelineDescriptor);

    const std::string& passName = std::filesystem::path(path).stem().string();

    setName(passName + "_CompositePass");
    setBindPoint(vk::PipelineBindPoint::eGraphics);
    setDepthAttachment(frameResources.getDepthBufferAttachment());

    const VulkanMesh* fullscreenMesh = meshManager.allocateMesh(VulkanMesh::makeFullscreenTriangle());

    auto fullscreenDraw = std::make_unique<VulkanDrawCall>();
    fullscreenDraw->setMesh(fullscreenMesh);
    addDrawCall(std::move(fullscreenDraw));

    return true;
}
