#include "MeshRenderPass.h"

#include "core/debug/ErrorHandling.h"

bool MeshRenderPass::create(
    const std::string&           path,
    const VulkanFrameResources&  frameResources,
    const VulkanRenderResources& renderResources,
    VulkanRenderObjectManager&   renderObjectManager,
    VulkanShaderProgramManager&  shaderProgramManager,
    std::string&                 errorMessage
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
    setDepthAttachment(renderResources.getDepthBufferAttachment());

    for (const auto& renderObject : renderObjectManager.getRenderObjects()) {
        // Each submesh requires its own draw call
        for (const auto& submesh : renderObject->submeshes) {
            auto verticesDraw = std::make_unique<VulkanDrawCallWithPushConstants>();

            verticesDraw->setMesh(submesh.mesh);
            verticesDraw->addDescriptorSets(submesh.descriptorSets);
            verticesDraw->setPushConstant("object", &renderObject->data);

            addDrawCall(std::move(verticesDraw));
        }
    }

    return true;
}
