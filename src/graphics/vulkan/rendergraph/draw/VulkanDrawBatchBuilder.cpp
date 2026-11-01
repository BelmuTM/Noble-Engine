#include "VulkanDrawBatchBuilder.h"

void VulkanDrawBatchBuilder::build(const std::vector<VulkanRenderItem>& renderItems, const std::uint32_t indirectionOffset) {
    _drawBatchMap.clear();
    _indirectionData.clear();
    _builtDrawBatches.clear();

    // Group by {mesh, material}
    for (const auto& [renderMesh, instanceHandle] : renderItems) {
        const VulkanDrawBatch key(renderMesh.mesh, renderMesh.material);

        _drawBatchMap[key].push_back(instanceHandle);
    }

    // Flatten into indirection data and built batches
    std::uint32_t firstInstance = indirectionOffset;

    for (const auto& [key, handles] : _drawBatchMap) {
        for (const auto& [objectIndex] : handles) {
            _indirectionData.push_back(objectIndex);
        }

        _builtDrawBatches.push_back(VulkanBuiltDrawBatch{
            .batch         = key,
            .firstInstance = firstInstance,
            .instanceCount = static_cast<std::uint32_t>(handles.size())
        });

        firstInstance += static_cast<std::uint32_t>(handles.size());
    }
}
