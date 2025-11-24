#include "MeshRenderPass.h"

#include "core/debug/ErrorHandling.h"

bool MeshRenderPass::create(
    const std::string&          path,
    const VulkanFrameResources& frameResources,
    VulkanRenderObjectManager&  renderObjectManager,
    VulkanShaderProgramManager& shaderProgramManager,
    std::string&                errorMessage
) {
    TRY(shaderProgramManager.load(getShaderProgram(), path, false, errorMessage));

    const std::string& passName = std::filesystem::path(path).stem().string();

    const VulkanPipelineDescriptor pipelineDescriptor{
        getShaderProgram(),
        {frameResources.getDescriptorManager().getLayout(), renderObjectManager.getDescriptorManager().getLayout()}
    };

    setName(passName + "_MeshRenderPass");
    setPipelineDescriptor(pipelineDescriptor);
    setBindPoint(vk::PipelineBindPoint::eGraphics);
    setDepthAttachment(frameResources.getDepthBufferAttachment());

    for (const auto& renderObject : renderObjectManager.getRenderObjects()) {
        addObjectDrawCall(renderObject.get());
    }

    return true;
}
