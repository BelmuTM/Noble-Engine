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

    for (const auto& renderObject : context.renderObjectManager.getRenderObjects()) {
        Mesh aabbMesh{};
        uint32_t vertexOffset = 0;

        // Draw each mesh's AABB
        for (const auto& mesh : renderObject->object->getModel().meshes) {
            const Math::AABB& aabb = mesh.getAABB();

            for (const auto& corner : aabb.getCorners()) {
                Vertex vertex{};
                vertex.position = corner;
                vertex.color    = Utility::instanceColor(&mesh);

                aabbMesh.addVertex(vertex);
            }

            const auto& indices = Math::AABB::getLineIndices(vertexOffset);
            aabbMesh.getIndices().insert(aabbMesh.getIndices().end(), indices.begin(), indices.end());

            vertexOffset += 8;
        }

        VulkanMesh* aabbMeshPtr = _meshManager.allocateMesh(aabbMesh);

        auto aabbDraw = std::make_unique<VulkanDrawCall>();
        aabbDraw
            ->setMesh(aabbMeshPtr)
            .setPushConstant("object", &renderObject->data);

        addDrawCall(std::move(aabbDraw));
    }

    TRY_deprecated(_meshManager.create(context.device, context.commandManager, errorMessage));
    TRY_deprecated(_meshManager.fillBuffers(errorMessage));

    return true;
}

void DebugPass::destroy() noexcept {
    VulkanRenderPass::destroy();

    _meshManager.destroy();
}
