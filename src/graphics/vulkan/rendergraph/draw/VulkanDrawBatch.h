#pragma once

#include "graphics/vulkan/resources/meshes/VulkanMesh.h"
#include "graphics/vulkan/resources/objects/VulkanMaterial.h"

struct VulkanDrawBatch {
    const VulkanMesh*     mesh     = nullptr;
    const VulkanMaterial* material = nullptr;

    bool operator==(const VulkanDrawBatch& other) const noexcept {
        return mesh == other.mesh && material == other.material;
    }
};

template<>
struct std::hash<VulkanDrawBatch> {
    std::size_t operator()(const VulkanDrawBatch& batch) const noexcept {
        const std::size_t h1 = std::hash<const VulkanMesh*>{}(batch.mesh);
        const std::size_t h2 = std::hash<const VulkanMaterial*>{}(batch.material);
        return h1 ^ (h2 << 1);
    }
};
