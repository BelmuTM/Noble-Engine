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
    TRY_ASSIGN(_indirectionBuffer, storageBufferManager.allocateBuffer(MAX_DRAWS * sizeof(uint32_t)));

    TRY(_descriptorManager.allocate(_indirectionDescriptors));

    _indirectionDescriptors->updatePerFrameSSBODescriptorSets(*_indirectionBuffer, 0);

    return {};
}

void VulkanFrameCuller::destroy() noexcept {
    _descriptorManager.destroy();
}

Expected<void> VulkanFrameCuller::cull(
    const VulkanRenderObjectManager& renderObjectManager, const FrameUniforms& uniforms
) {
    const glm::mat4& viewProjectionMatrix = uniforms.projectionMatrix * uniforms.viewMatrix;

    const std::array<Math::Plane, 6>& frustumPlanes = FrustumCuller::getFrustumPlanes(viewProjectionMatrix);

    _visibleItems.clear();

    for (const auto& renderObject : renderObjectManager.getRenderObjects()) {
        for (const auto& renderMesh : renderObject->meshes) {

            Math::AABB worldAABB = renderMesh.mesh->getAABB().transform(renderObject->object->getModelMatrix());

            const bool visible = FrustumCuller::testVisibility(worldAABB, frustumPlanes);

            if (visible) {
                _visibleItems.emplace_back(renderMesh, renderObject->instanceHandle);
            }
        }
    }

    return {};
}
