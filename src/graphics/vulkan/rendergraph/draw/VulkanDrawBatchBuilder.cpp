#include "VulkanDrawBatchBuilder.h"

#include "graphics/vulkan/resources/materials/VulkanMaterial.h"
#include "graphics/vulkan/resources/meshes/VulkanMesh.h"

#include <algorithm>

Expected<void> VulkanDrawBatchBuilder::create(
    const VulkanDevice&         device,
    VulkanStorageBufferManager& storageBufferManager,
    const VulkanStorageBuffer*  objectBuffer,
    const std::uint32_t         framesInFlight
) noexcept {
    // Create descriptor manager
    TRY(_descriptorManager.create(device.getLogicalDevice(), getDescriptorScheme(), framesInFlight, 1));

    // Create indirection buffer
    TRY_ASSIGN(_indirectionBuffer, storageBufferManager.allocateBuffer(MAX_INSTANCES * sizeof(uint32_t)));

    // Allocate descriptors
    TRY(_descriptorManager.allocate(_descriptors));

    // Update descriptors
    _descriptors->updatePerFrameSSBODescriptorSets(*objectBuffer, 0);
    _descriptors->updatePerFrameSSBODescriptorSets(*_indirectionBuffer, 1);

    return {};
}

void VulkanDrawBatchBuilder::destroy() noexcept {
    _descriptorManager.destroy();
}

namespace {

std::uint64_t makeSortKey(const VulkanRenderMesh& renderMesh) noexcept {
    const std::uint32_t materialId = renderMesh.material ? renderMesh.material->getId() : 0xFFFFFFFFu;
    // Material-major key, 32 highest significant bits hold material ID, the remaining 32 hold mesh first index
    return (static_cast<std::uint64_t>(materialId) << 32) | renderMesh.mesh->getFirstIndex();
}

VulkanDrawCommand makeDrawCommand(
    const VulkanRenderMesh& renderMesh,
    const std::uint32_t     instanceCount,
    const std::uint32_t     firstInstance
) {
    const VulkanMesh& mesh = *renderMesh.mesh;
    return {
        .material = renderMesh.material,
        .command  = vk::DrawIndexedIndirectCommand{
            static_cast<std::uint32_t>(mesh.getIndices().size()),
            instanceCount,
            mesh.getFirstIndex(),
            mesh.getVertexOffset(),
            firstInstance
        }
    };
}

}

Expected<void> VulkanDrawBatchBuilder::build(
    const std::span<const std::unique_ptr<VulkanGraphicsPass>> passes,
    const std::span<const VulkanRenderItem>                    allItems,
    const std::span<const VulkanRenderItem>                    visibleItems,
    const std::uint32_t                                        frameIndex
) {

    _drawBatches.resize(passes.size());
    _indirection.clear();

    for (const auto& pass : passes) {
        const auto& desc = pass->getGraphicsPassDescriptor();

        auto& batches = _drawBatches[pass->base().getIndex()];
        batches.clear();

        if (desc.drawFlags.empty()) continue;

        _entries.clear();

        const auto renderItems = desc.cullMode == VulkanGraphicsPassCullMode::None
            ? allItems
            : visibleItems;

        for (const auto& [renderMesh, handle] : renderItems) {
            if (
                !renderMesh.mesh ||
                renderMesh.mesh->getIndices().empty() ||
                !renderMesh.drawFlags.intersects(desc.drawFlags)
            ) continue;

            _entries.emplace_back(makeSortKey(renderMesh), handle.objectIndex, renderMesh);
        }

        // Key sorting
        std::ranges::sort(_entries, [](const Entry& first, const Entry& second) {
            return first.key != second.key ? first.key < second.key : first.objectIndex < second.objectIndex;
        });

        const auto base = static_cast<std::uint32_t>(_indirection.size());

        if (base + _entries.size() > MAX_INSTANCES) {
            return VK_FAIL("Failed to build draw command batches: exceeded maximum instances.");
        }

        // Run-length encoding
        for (std::size_t i = 0; i < _entries.size();) {

            std::size_t j = i;

            while (j < _entries.size() && _entries[j].key == _entries[i].key) {
                _indirection.push_back(_entries[j++].objectIndex);
            }

            batches.push_back(
                makeDrawCommand(
                    _entries[i].renderMesh,
                    static_cast<std::uint32_t>(j - i),
                    base + static_cast<std::uint32_t>(i)
                )
            );

            i = j;
        }
    }

    if (!_indirection.empty()) {
        _indirectionBuffer->updateArrayMemory(frameIndex, _indirection);
    }

    return {};
}
