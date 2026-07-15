#include "VulkanMeshRenderPass.h"

Expected<void> VulkanMeshRenderPass::create(const VulkanMeshRenderPassCreateContext& context) {

    for (const auto& renderObject : context.renderObjectManager.getRenderObjects()) {
        // Each submesh requires its own draw call
        for (const auto& renderMesh : renderObject->meshes) {
            emplaceDrawCall()
                .setName(renderObject->object->getModel().name)
                .setRenderMesh(renderMesh)
                .setInstanceHandle(renderObject->instanceHandle)
                .setModelMatrix(renderObject->object->getModelMatrix());
        }
    }

    return {};
}
