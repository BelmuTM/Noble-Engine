#include "MeshRenderPass.h"

#include "core/debug/ErrorHandling.h"

bool MeshRenderPass::create(
    const std::string&          path,
    VulkanShaderProgramManager& shaderProgramManager,
    const VulkanImageManager&   imageManager,
    VulkanFrameResources&       frameResources,
    VulkanRenderObjectManager&  renderObjectManager,
    std::string&                errorMessage
) {
    VulkanShaderProgram* program;
    TRY(shaderProgramManager.load(program, path, false, errorMessage));

    // TO-DO: Use reflections for push constants
    vk::PushConstantRange pushConstantRange{};
    pushConstantRange
        .setStageFlags(program->getStageFlags())
        .setOffset(0)
        .setSize(objectDataGPUSize);

    const VulkanPipelineDescriptor pipelineDescriptor{
        program,
        {frameResources.getDescriptorManager().getLayout(), renderObjectManager.getDescriptorManager().getLayout()},
        {pushConstantRange}
    };

    setPipelineDescriptor(pipelineDescriptor);

    TRY(createColorAttachments(*program, imageManager, frameResources, errorMessage));

    const std::string& passName = std::filesystem::path(path).stem().string();

    setName(passName + "_MeshRenderPass");
    setBindPoint(vk::PipelineBindPoint::eGraphics);
    setDepthAttachment(frameResources.getDepthBufferAttachment());

    for (const auto& renderObject : renderObjectManager.getRenderObjects()) {
        addObjectDrawCall(renderObject.get());
    }

    return true;
}
