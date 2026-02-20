#include "VulkanFrameDraws.h"

#include "core/render/FrustumCuller.h"

bool VulkanFrameDraws::create(const VulkanFrameResources& frameResources, std::string& errorMessage) noexcept {
    _frameResources = &frameResources;

    return true;
}

void VulkanFrameDraws::destroy() noexcept {
    _visibleDrawCalls.clear();

    _frameResources = nullptr;
}

void VulkanFrameDraws::cullDraws(const std::vector<std::unique_ptr<VulkanRenderPass>>& passes) {
    const FrameUniforms& uniforms             = _frameResources->getUniforms();
    const glm::mat4      viewProjectionMatrix = uniforms.projectionMatrix * uniforms.viewMatrix;

    const std::array<Math::Plane, 6>& frustumPlanes = FrustumCuller::getFrustumPlanes(viewProjectionMatrix);

    _visibleDrawCalls.clear();
    _visibleDrawCalls.reserve(passes.size());

    for (const auto& pass : passes) {
        auto& visibleDraws = _visibleDrawCalls[pass.get()];
        visibleDraws.clear();

        for (const auto& drawCall : pass->getDrawCalls()) {

            bool visible = true;

            if (pass->getType() == VulkanRenderPassType::MeshRender) {
                Math::AABB worldAABB = drawCall->getMesh()->getAABB().transform(drawCall->getObject()->data.modelMatrix);

                visible = FrustumCuller::testVisibility(worldAABB, frustumPlanes);
            }

            if (visible) {
                visibleDraws.push_back(drawCall.get());
            }
        }
    }
}
