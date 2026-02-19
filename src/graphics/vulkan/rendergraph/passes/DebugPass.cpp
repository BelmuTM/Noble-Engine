#include "DebugPass.h"

#include "core/debug/ErrorHandling.h"

DebugPass::~DebugPass() {
    _meshManager.destroy();
}

bool DebugPass::create(
    const std::string&            path,
    const DebugPassCreateContext& context,
    std::string&                  errorMessage
) {
    TRY(context.shaderProgramManager.load(getShaderProgram(), path, errorMessage));

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

        for (const auto& mesh : renderObject->object->getModel().meshes) {
            const Math::AABB& aabb = mesh.getAABB();

            for (const auto& corner : aabb.getCorners()) {
                Vertex vertex{};
                vertex.position = corner;
                vertex.color    = glm::vec3(0.0f, 1.0f, 1.0f);

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

    TRY(_meshManager.create(context.device, context.commandManager, errorMessage));
    TRY(_meshManager.fillBuffers(errorMessage));

    return true;
}
