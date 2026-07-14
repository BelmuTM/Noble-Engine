#include "VulkanMeshRenderPass.h"

constexpr auto hihi = glm::vec3(1.0f, 0.0f, 1.0f);

Expected<void> VulkanMeshRenderPass::create(const VulkanMeshRenderPassCreateContext& context) {

    for (const auto& renderObject : context.renderObjectManager.getRenderObjects()) {
        // Each submesh requires its own draw call
        for (const auto& renderMesh : renderObject->meshes) {
            emplaceDrawCall()
                .setName(renderObject->object->getModel().name)
                .setRenderMesh(renderMesh)
                .setInstanceHandle(renderObject->instanceHandle)
                .setModelMatrix(renderObject->object->getModelMatrix());
                //.setPushConstant("meow", &hihi);
        }
    }

    return {};
}
