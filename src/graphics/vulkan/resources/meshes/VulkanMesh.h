#pragma once
#ifndef NOBLEENGINE_VULKANMESH_H
#define NOBLEENGINE_VULKANMESH_H

#include "core/resources/models/Mesh.h"

class VulkanMesh final : public Mesh {
public:
    VulkanMesh()  = default;
    ~VulkanMesh() = default;

    explicit VulkanMesh(const Mesh& baseMesh) {
        this->_vertices = baseMesh.getVertices();
        this->_indices  = baseMesh.getIndices();
        this->_material = baseMesh.getMaterial();
    }

    VulkanMesh(const VulkanMesh&)            noexcept = default;
    VulkanMesh& operator=(const VulkanMesh&) noexcept = default;

    VulkanMesh(VulkanMesh&&)            noexcept = default;
    VulkanMesh& operator=(VulkanMesh&&) noexcept = default;

    [[nodiscard]] bool isBufferless() const noexcept { return _bufferless; }
    void setBufferless(const bool bufferless) noexcept { _bufferless = bufferless; }

    [[nodiscard]] size_t getVertexOffset() const noexcept { return _vertexOffset; }
    [[nodiscard]] size_t getIndexOffset() const noexcept { return _indexOffset; }

    void setVertexOffset(const size_t offset) noexcept {
        _vertexOffset = offset;
    }

    void setIndexOffset(const size_t offset) noexcept {
        _indexOffset = offset;
    }

    static VulkanMesh makeFullscreenTriangle();

private:
    bool _bufferless = false;

    size_t _vertexOffset = 0;
    size_t _indexOffset  = 0;
};

#endif // NOBLEENGINE_VULKANMESH_H
