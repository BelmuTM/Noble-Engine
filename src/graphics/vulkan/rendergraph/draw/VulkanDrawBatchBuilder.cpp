#include "VulkanDrawBatchBuilder.h"

#include <ranges>

void VulkanDrawBatchBuilder::build(const std::vector<const VulkanDrawCall*>& drawCalls) {
    _drawBatchMap.clear();
    _indirectionData.clear();
    _builtDrawBatches.clear();

    // Group by {mesh, material}, store representative
    for (const auto* drawCall : drawCalls) {
        const auto& [mesh, material] = drawCall->getRenderMesh();

        auto& [rep, handles] = _drawBatchMap[{mesh, material}];
        if (!rep) rep = drawCall;

        handles.push_back(drawCall->getInstanceHandle());
    }

    // Flatten into indirection data and built batches
    uint32_t firstInstance = 0;

    for (const auto& [rep, handles] : _drawBatchMap | std::views::values) {

        for (const auto& [objectIndex] : handles) {
            _indirectionData.push_back(objectIndex);
        }

        _builtDrawBatches.push_back({rep, firstInstance, static_cast<uint32_t>(handles.size())});

        firstInstance += static_cast<uint32_t>(handles.size());
    }
}
