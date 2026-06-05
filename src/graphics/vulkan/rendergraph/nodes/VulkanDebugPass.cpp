#include "VulkanDebugPass.h"

#include "common/Utility.h"

Expected<void> VulkanDebugPass::create(const VulkanDebugPassCreateContext& context) {

    getPipelineLayoutDescriptor().descriptorLayouts = {
        context.frameResources.getDescriptorManager().getLayout(),
        context.renderObjectManager.getDescriptorManager().getLayout(),
        context.frameCuller.getDescriptorManager().getLayout()
    };

    std::size_t meshHash = 0;

    for (const auto& renderObject : context.renderObjectManager.getRenderObjects()) {
        Mesh aabbMesh{};
        std::uint32_t vertexOffset = 0;

        // Draw each mesh's AABB
        for (const auto& mesh : renderObject->object->getModel().meshes) {

            HashUtils::combine(meshHash, &mesh);

            for (const auto& corner : mesh.getAABB().getCorners()) {
                aabbMesh.addVertex(Vertex{corner});
            }

            const auto& indices = Math::AABB::getLineIndices(vertexOffset);
            aabbMesh.getIndices().insert(aabbMesh.getIndices().end(), indices.begin(), indices.end());

            aabbMesh.setAABB(mesh.getAABB());

            vertexOffset += 8;
        }

        renderObject->gpuData.debugColor = Utility::instanceColor(meshHash);

        emplaceDrawCall()
            .setName(renderObject->object->getModel().name + "_Debug")
            .setRenderMesh({context.meshManager.allocateMesh(aabbMesh)})
            .setInstanceHandle(renderObject->instanceHandle)
            .setModelMatrix(renderObject->object->getModelMatrix())
            .addDescriptorSets(context.renderObjectManager.getDescriptorSets())
            .addDescriptorSets(context.frameCuller.getDescriptorSets());
    }

    return {};
}
