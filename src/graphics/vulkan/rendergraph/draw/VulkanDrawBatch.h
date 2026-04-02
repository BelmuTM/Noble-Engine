#pragma once

#include "graphics/vulkan/resources/materials/VulkanMaterial.h"
#include "graphics/vulkan/resources/meshes/VulkanMesh.h"

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
        std::size_t hash = 0;

        HashUtils::combine(hash, batch.mesh);
        HashUtils::combine(hash, batch.material);

        return hash;
    }
};
