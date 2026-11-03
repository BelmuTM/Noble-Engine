#include "VulkanFrameCuller.h"

#include "core/render/FrameUniforms.h"
#include "core/render/FrustumCuller.h"

#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

void VulkanFrameCuller::cull(const VulkanRenderObjectManager& renderObjectManager, const FrameUniforms& uniforms) {

    const glm::mat4& viewProjectionMatrix = uniforms.projectionMatrix * uniforms.viewMatrix;

    const std::array<Math::Plane, 6>& frustumPlanes = FrustumCuller::getFrustumPlanes(viewProjectionMatrix);

    _visibleItems.clear();

    for (const auto& renderObject : renderObjectManager.getRenderObjects()) {
        for (const auto& renderMesh : renderObject->meshes) {

            Math::AABB worldAABB = renderMesh.mesh->getAABB().transform(renderObject->object->getModelMatrix());

            if (FrustumCuller::testVisibility(worldAABB, frustumPlanes)) {
                _visibleItems.emplace_back(renderMesh, renderObject->instanceHandle);
            }
        }
    }
}
