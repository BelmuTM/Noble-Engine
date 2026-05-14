#include "MeshRenderPass.h"

Expected<void> MeshRenderPass::create(const std::string& path, const MeshRenderPassCreateContext& context) {
    TRY(context.shaderProgramManager.load(getShaderProgram(), path));

    const std::string& passName = std::filesystem::path(path).stem().string();

    const VulkanPipelineDescriptor pipelineDescriptor{
        getShaderProgram(),
        {
            context.frameResources.getDescriptorManager().getLayout(),
            context.renderObjectManager.getDescriptorManager().getLayout(),
            context.frameCuller.getDescriptorManager().getLayout(),
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
        for (const auto& renderMesh : renderObject->meshes) {
            emplaceDrawCall()
                .setName(renderObject->object->getModel().name)
                .setRenderMesh(renderMesh)
                .setInstanceHandle(renderObject->instanceHandle)
                .setModelMatrix(renderObject->object->getModelMatrix())
                .addDescriptorSets(context.renderObjectManager.getDescriptorSets())
                .addDescriptorSets(context.frameCuller.getDescriptorSets())
                .addDescriptorSets(renderMesh.material->getDescriptorSets());
                //.setPushConstant("object", &renderObject->gpuData);
        }
    }

    return {};
}
