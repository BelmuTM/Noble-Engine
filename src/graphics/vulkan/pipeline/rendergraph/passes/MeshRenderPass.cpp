#include "MeshRenderPass.h"

#include "core/debug/ErrorHandling.h"

bool MeshRenderPass::create(
    const std::string&          path,
    const VulkanImageManager&   imageManager,
    VulkanFrameResources&       frameResources,
    VulkanRenderResources&      renderResources,
    VulkanRenderObjectManager&  renderObjectManager,
    VulkanShaderProgramManager& shaderProgramManager,
    std::string&                errorMessage
) {
    VulkanShaderProgram* program;
    TRY(shaderProgramManager.load(program, path, false, errorMessage));

    TRY(createColorAttachments(*program, imageManager, frameResources, renderResources, errorMessage));

    VulkanPipelineDescriptor pipelineDescriptor{
        program,
        {frameResources.getDescriptorManager().getLayout(), renderObjectManager.getDescriptorManager().getLayout()}
    };

    TRY(renderResources.allocateDescriptors(pipelineDescriptor, program->getDescriptorSchemes(), errorMessage));

    setPipelineDescriptor(pipelineDescriptor);

    const std::string& passName = std::filesystem::path(path).stem().string();

    setName(passName + "_MeshRenderPass");
    setBindPoint(vk::PipelineBindPoint::eGraphics);
    setDepthAttachment(frameResources.getDepthBufferAttachment());

    for (const auto& renderObject : renderObjectManager.getRenderObjects()) {
        addObjectDrawCall(renderObject.get());
    }

    return true;
}
