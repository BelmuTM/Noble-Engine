#include "VulkanFrameCuller.h"

#include "core/render/FrustumCuller.h"

Expected<void> VulkanFrameCuller::create(
    const VulkanDevice&         device,
    VulkanStorageBufferManager& storageBufferManager,
    const std::uint32_t         framesInFlight
) noexcept {
    // Create descriptor manager
    TRY(_descriptorManager.create(device.getLogicalDevice(), getDescriptorScheme(), framesInFlight, MAX_DRAWS));

    // Create indirection buffer
    TRY_ASSIGN(_indirectionBuffer, storageBufferManager.allocateBuffer(MAX_DRAWS * sizeof(uint32_t)));

    TRY(_descriptorManager.allocate(_indirectionDescriptors));

    _indirectionDescriptors->updatePerFrameSSBODescriptorSets(*_indirectionBuffer, 0);

    return {};
}

void VulkanFrameCuller::destroy() noexcept {
    _descriptorManager.destroy();
}

Expected<void> VulkanFrameCuller::cull(
    const std::vector<std::unique_ptr<VulkanRenderPass>>& passes, const FrameUniforms& uniforms
) {
    const glm::mat4& viewProjectionMatrix = uniforms.projectionMatrix * uniforms.viewMatrix;

    const std::array<Math::Plane, 6>& frustumPlanes = FrustumCuller::getFrustumPlanes(viewProjectionMatrix);

    _visibleDrawCalls.clear();
    _visibleDrawCalls.reserve(passes.size());

    std::uint32_t currentIndirectionOffset = 0;

    for (const auto& pass : passes) {
        auto& visibleDraws = _visibleDrawCalls[pass.get()];

        visibleDraws.clear();

        // Check visibility for passes with culling enabled
        if (pass->getPassDescriptor().cullMode == VulkanRenderPassCullMode::None) {
            for (auto& draw : pass->getDrawCalls())
                visibleDraws.push_back(&draw);

        } else {
            for (auto& drawCall : pass->getDrawCalls()) {

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

        // Keep track of the indirection offset
        _indirectionOffsets[pass.get()] = currentIndirectionOffset;

        currentIndirectionOffset += static_cast<std::uint32_t>(pass->getDrawCalls().size());

        if (currentIndirectionOffset > MAX_DRAWS) {
            return VK_FAIL("Failed to cull frame: exceeded maximum draws.");
        }
    }

    return {};
}
