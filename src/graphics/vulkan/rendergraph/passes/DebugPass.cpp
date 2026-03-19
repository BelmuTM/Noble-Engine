#include "DebugPass.h"

#include "common/Utility.h"
#include "core/debug/ErrorHandling.h"

bool DebugPass::create(
    const std::string&            path,
    const DebugPassCreateContext& context,
    std::string&                  errorMessage
) {
    TRY_deprecated(context.shaderProgramManager.load(getShaderProgram(), path, errorMessage));

    const std::string& passName = std::filesystem::path(path).stem().string();

    const VulkanPipelineDescriptor pipelineDescriptor{
        getShaderProgram(),
        {
            context.frameResources.getDescriptorManager().getLayout(),
            context.renderObjectManager.getDescriptorManager().getLayout()
        }
    };

    setType(VulkanRenderPassType::Debug);
    setName(passName + "_DebugPass");
    setPipelineDescriptor(pipelineDescriptor);
    setBindPoint(vk::PipelineBindPoint::eGraphics);

    uint64_t meshSeed = INT64_MAX;

    for (const auto& renderObject : context.renderObjectManager.getRenderObjects()) {
        Mesh aabbMesh{};
        uint32_t vertexOffset = 0;

        // Draw each mesh's AABB
        for (const auto& mesh : renderObject->object->getModel().meshes) {

            meshSeed ^= reinterpret_cast<uint64_t>(&mesh);

            for (const auto& corner : mesh.getAABB().getCorners()) {
                Vertex vertex{};
                vertex.position = corner;
                vertex.color    = Utility::instanceColor(meshSeed);

                aabbMesh.addVertex(vertex);
            }

            const auto& indices = Math::AABB::getLineIndices(vertexOffset);
            aabbMesh.getIndices().insert(aabbMesh.getIndices().end(), indices.begin(), indices.end());

            vertexOffset += 8;
        }

        VulkanMesh* aabbMeshPtr = _meshManager.allocateMesh(aabbMesh);

        emplaceDrawCall()
            .setMesh(aabbMeshPtr)
            .setPushConstant("object", &renderObject->data);
    }

    TRY_deprecated(_meshManager.create(context.device, context.commandManager, errorMessage));
    TRY_deprecated(_meshManager.fillBuffers(errorMessage));

    return true;
}

void DebugPass::destroy() noexcept {
    VulkanRenderPass::destroy();

    _meshManager.destroy();
}
