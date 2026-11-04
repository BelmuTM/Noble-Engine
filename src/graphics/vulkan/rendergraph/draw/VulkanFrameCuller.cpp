#include "VulkanFrameCuller.h"

#include "core/render/FrameUniforms.h"
#include "core/render/FrustumCuller.h"

#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

void VulkanFrameCuller::cull(const VulkanRenderObjectManager& renderObjectManager, const FrameUniforms& uniforms) {

    const glm::mat4& viewProjectionMatrix = uniforms.projectionMatrix * uniforms.viewMatrix;

    const std::array<Math::Plane, 6>& frustumPlanes = FrustumCuller::getFrustumPlanes(viewProjectionMatrix);

    _visibleItems.clear();

    for (const auto& renderObject : renderObjectManager.getRenderObjects()) {

        const glm::mat4& model = renderObject->getObject().getModelMatrix();

        for (const auto& renderMesh : renderObject->getMeshes()) {

            const Math::OBB& obb = renderMesh.mesh->getOBB();

            const glm::mat4 meshTransform = glm::transpose(model * obb.transformMatrix);

            // Transform world-space frustum planes to local model space
            std::array<Math::Plane, 6> localPlanes{};

            for (std::size_t i = 0; i < 6; i++) {
                const glm::vec4 plane = meshTransform * glm::vec4(frustumPlanes[i].normal, frustumPlanes[i].d);
                localPlanes[i] = {.normal = glm::vec3(plane), .d = plane.w};
            }

            if (FrustumCuller::testVisibility(obb, localPlanes)) {
                _visibleItems.emplace_back(renderMesh, renderObject->getInstanceHandle());
            }
        }
    }
}
