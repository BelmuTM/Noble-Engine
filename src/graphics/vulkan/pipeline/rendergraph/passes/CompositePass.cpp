#include "CompositePass.h"

#include "core/debug/ErrorHandling.h"

bool CompositePass::create(
    const std::string&          path,
    VulkanShaderProgramManager& shaderProgramManager,
    const VulkanImageManager&   imageManager,
    const VulkanFrameResources& frameResources,
    std::string&                errorMessage
) {
    VulkanShaderProgram* program;
    TRY(shaderProgramManager.load(program, path, false, errorMessage));

    const VulkanPipelineDescriptor pipelineDescriptor{
        program,
        {frameResources.getDescriptorManager().getLayout()},
        {}
    };

    setPipelineDescriptor(pipelineDescriptor);

    const std::string& passName = std::filesystem::path(path).stem().string();

    setName(passName + "_CompositePass");
    setBindPoint(vk::PipelineBindPoint::eGraphics);
    setDepthAttachment(frameResources.getDepthBufferAttachment());

    auto fullscreenDraw = std::make_unique<VulkanDrawCall>();
    fullscreenDraw->setMesh(VulkanMesh::makeFullscreenTriangle());
    addDrawCall(std::move(fullscreenDraw));

    return true;
}
