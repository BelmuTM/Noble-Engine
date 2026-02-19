#include "MeshRenderPass.h"

#include "core/debug/ErrorHandling.h"

bool MeshRenderPass::create(
    const std::string&                 path,
    const MeshRenderPassCreateContext& context,
    std::string&                       errorMessage
) {
    TRY(context.shaderProgramManager.load(getShaderProgram(), path, errorMessage));

    const std::string& passName = std::filesystem::path(path).stem().string();

    const VulkanPipelineDescriptor pipelineDescriptor{
        getShaderProgram(),
        {
            context.frameResources.getDescriptorManager().getLayout(),
            context.renderObjectManager.getDescriptorManager().getLayout()
        }
    };

    setType(VulkanRenderPassType::MeshRender);
    setName(passName + "_MeshRenderPass");
    setPipelineDescriptor(pipelineDescriptor);
    setBindPoint(vk::PipelineBindPoint::eGraphics);
    setDepthAttachment(context.renderResources.getDepthBufferAttachment());

    for (const auto& renderObject : context.renderObjectManager.getRenderObjects()) {
        // Each submesh requires its own draw call
        for (const auto& submesh : renderObject->submeshes) {
            auto verticesDraw = std::make_unique<VulkanDrawCall>();
            verticesDraw
                ->setMesh(submesh.mesh)
                .setOwner(renderObject.get())
                .addDescriptorSets(submesh.descriptorSets)
                .setPushConstant("object", &renderObject->data);

            addDrawCall(std::move(verticesDraw));
        }
    }

    return true;
}
