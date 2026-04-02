#include "MeshRenderPass.h"

#include "core/debug/ErrorHandling.h"

bool MeshRenderPass::create(
    const std::string&                 path,
    const MeshRenderPassCreateContext& context,
    std::string&                       errorMessage
) {
    TRY_BOOL(context.shaderProgramManager.load(getShaderProgram(), path, errorMessage));

    const std::string& passName = std::filesystem::path(path).stem().string();

    const VulkanPipelineDescriptor pipelineDescriptor{
        getShaderProgram(),
        {
            context.frameResources.getDescriptorManager().getLayout(),
            context.materialManager.getDescriptorManager().getLayout()
        }
    };

    setType(VulkanRenderPassType::MeshRender);
    setName(passName + "_MeshRenderPass");
    setPipelineDescriptor(pipelineDescriptor);
    setBindPoint(vk::PipelineBindPoint::eGraphics);
    setDepthAttachment(context.renderResources.getDepthBufferAttachment());

    for (const auto& renderObject : context.renderObjectManager.getRenderObjects()) {
        // Each submesh requires its own draw call
        for (const auto& [mesh, material] : renderObject->meshes) {
            emplaceDrawCall()
                .setName(renderObject->object->getModel().name)
                .setMesh(mesh)
                .setModelMatrix(renderObject->object->getModelMatrix())
                .addDescriptorSets(material->getDescriptorSets())
                .setPushConstant("object", &renderObject->gpuData);
        }
    }

    return true;
}
