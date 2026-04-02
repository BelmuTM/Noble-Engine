#include "VulkanDrawBatchBuilder.h"

void VulkanDrawBatchBuilder::build() {
    for (const auto& renderObject : _renderObjectManager.getRenderObjects()) {
        for (const auto& [mesh, material] : renderObject->meshes) {
            VulkanDrawBatch batch{mesh, &material};

            _batchMap[batch].push_back(renderObject->instanceHandle);
        }
    }
}
