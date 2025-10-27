#include "VulkanMesh.h"

VulkanMesh::VulkanMesh(const VulkanMesh& other) noexcept {
    _vertexOffset = other._vertexOffset;
    _indexOffset  = other._indexOffset;
    _vertices     = other._vertices;
    _indices      = other._indices;
}

VulkanMesh& VulkanMesh::operator=(const VulkanMesh& other) noexcept {
    if (this != &other) {
        _vertexOffset = other._vertexOffset;
        _indexOffset  = other._indexOffset;
        _vertices     = other._vertices;
        _indices      = other._indices;
    }
    return *this;
}
