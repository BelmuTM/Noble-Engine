#include "VulkanMesh.h"

VulkanMesh::VulkanMesh(const VulkanMesh& other) noexcept {
    _vertexOffset = other._vertexOffset;
    _indexOffset  = other._indexOffset;
}

VulkanMesh& VulkanMesh::operator=(const VulkanMesh& other) noexcept {
    if (this != &other) {
        _vertexOffset = other._vertexOffset;
        _indexOffset  = other._indexOffset;
    }
    return *this;
}
