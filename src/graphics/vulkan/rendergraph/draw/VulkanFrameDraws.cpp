#include "VulkanFrameDraws.h"

#include "core/render/FrustumCuller.h"

void VulkanFrameDraws::cullDraws(
    const std::vector<std::unique_ptr<VulkanRenderPass>>& passes, const FrameUniforms& uniforms
) {
    const glm::mat4& viewProjectionMatrix = uniforms.projectionMatrix * uniforms.viewMatrix;

    const std::array<Math::Plane, 6>& frustumPlanes = FrustumCuller::getFrustumPlanes(viewProjectionMatrix);

    _visibleDrawCalls.clear();
    _visibleDrawCalls.reserve(passes.size());

    for (const auto& pass : passes) {
        auto& visibleDraws = _visibleDrawCalls[pass.get()];
        visibleDraws.clear();

        for (const auto& drawCall : pass->getDrawCalls()) {

            bool visible = true;

            if (drawCall.getModelMatrix()) {
                Math::AABB worldAABB = drawCall.getMesh()->getAABB().transform(*drawCall.getModelMatrix());

                visible = FrustumCuller::testVisibility(worldAABB, frustumPlanes);
            }

            if (visible) {
                visibleDraws.push_back(&drawCall);
            }
        }
    }
}
