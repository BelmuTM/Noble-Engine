#include "VulkanMeshRenderPass.h"

constexpr auto hihi = glm::vec3(1.0f, 0.0f, 1.0f);

Expected<void> VulkanMeshRenderPass::create(const VulkanMeshRenderPassCreateContext& context) {

    getPipelineLayoutDescriptor().descriptorLayouts = {
        context.frameResources.getDescriptorManager().getLayout(),
        context.renderObjectManager.getDescriptorManager().getLayout(),
        context.frameCuller.getDescriptorManager().getLayout(),
        context.materialManager.getDescriptorManager().getLayout()
    };

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
                //.setPushConstant("meow", &hihi);
        }
    }

    return {};
}
