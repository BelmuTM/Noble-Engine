#include "VulkanFrameCuller.h"

#include "core/render/FrustumCuller.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

Expected<void> VulkanFrameCuller::create(
    const VulkanDevice&         device,
    VulkanStorageBufferManager& storageBufferManager,
    const std::uint32_t         framesInFlight
) noexcept {
    // Create descriptor manager
    TRY(_descriptorManager.create(device.getLogicalDevice(), getDescriptorScheme(), framesInFlight, MAX_DRAWS));

    // Create indirection buffer
    VK_TRY_ASSIGN(_indirectionBuffer, storageBufferManager.allocateBuffer(MAX_DRAWS * sizeof(uint32_t)));

    TRY(_descriptorManager.allocate(_indirectionDescriptors));

    _indirectionDescriptors->updatePerFrameSSBODescriptorSets(*_indirectionBuffer, 0);

    return {};
}

void VulkanFrameCuller::destroy() noexcept {
    _descriptorManager.destroy();
}

void VulkanFrameCuller::cull(
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
                Math::AABB worldAABB = drawCall.getRenderMesh().mesh->getAABB().transform(*drawCall.getModelMatrix());

                visible = FrustumCuller::testVisibility(worldAABB, frustumPlanes);
            }

            if (visible) {
                visibleDraws.push_back(&drawCall);
            }
        }
    }
}
