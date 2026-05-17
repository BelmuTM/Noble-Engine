#include "DebugPass.h"

#include "common/Utility.h"

Expected<void> DebugPass::create(const DebugPassCreateContext& context) {
    TRY(context.shaderProgramManager.load(getShaderProgram(), getPassDescriptor().programPath));

    const VulkanPipelineDescriptor pipelineDescriptor{
        getShaderProgram(),
        {
            context.frameResources.getDescriptorManager().getLayout()
        }
    };

    setPipelineDescriptor(pipelineDescriptor);
    setDepthAttachment(context.renderResources.getDepthBufferAttachment());

    std::size_t meshHash = 0;

    for (const auto& renderObject : context.renderObjectManager.getRenderObjects()) {
        Mesh aabbMesh{};
        std::uint32_t vertexOffset = 0;

        // Draw each mesh's AABB
        for (const auto& mesh : renderObject->object->getModel().meshes) {

            HashUtils::combine(meshHash, &mesh);

            for (const auto& corner : mesh.getAABB().getCorners()) {
                Vertex vertex{};
                vertex.position = corner;
                vertex.color    = Utility::instanceColor(meshHash);

                aabbMesh.addVertex(vertex);
            }

            const auto& indices = Math::AABB::getLineIndices(vertexOffset);
            aabbMesh.getIndices().insert(aabbMesh.getIndices().end(), indices.begin(), indices.end());

            vertexOffset += 8;
        }

        VulkanMesh* aabbMeshPtr = context.meshManager.allocateMesh(aabbMesh);

        emplaceDrawCall()
            .setName(renderObject->object->getModel().name + "_Debug")
            .setRenderMesh({aabbMeshPtr})
            .setModelMatrix(renderObject->object->getModelMatrix())
            .setPushConstant("object", &renderObject->gpuData);
    }

    return {};
}
