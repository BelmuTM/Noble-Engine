#include "DebugPass.h"

#include "common/Utility.h"

Expected<void> DebugPass::create(const std::string& path, const DebugPassCreateContext& context) {
    TRY(context.shaderProgramManager.load(getShaderProgram(), path));

    const std::string& passName = std::filesystem::path(path).stem().string();

    const VulkanPipelineDescriptor pipelineDescriptor{
        getShaderProgram(),
        {
            context.frameResources.getDescriptorManager().getLayout()
        }
    };

    setType(VulkanRenderPassType::Debug);
    setName(passName + "_DebugPass");
    setPipelineDescriptor(pipelineDescriptor);
    setBindPoint(vk::PipelineBindPoint::eGraphics);
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

        VulkanMesh* aabbMeshPtr = _meshManager.allocateMesh(aabbMesh);

        emplaceDrawCall()
            .setName(renderObject->object->getModel().name + "_Debug")
            .setRenderMesh({aabbMeshPtr})
            .setModelMatrix(renderObject->object->getModelMatrix())
            .setPushConstant("object", &renderObject->gpuData);
    }

    TRY(_meshManager.create(context.device, context.commandManager));
    TRY(_meshManager.fillBuffers());

    return {};
}

void DebugPass::destroy() noexcept {
    VulkanRenderPass::destroy();

    _meshManager.destroy();
}
