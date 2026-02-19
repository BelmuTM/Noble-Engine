#pragma once

#include "core/resources/models/Mesh.h"

#include "graphics/vulkan/core/memory/VulkanBuffer.h"

class VulkanMesh final : public Mesh {
public:
    VulkanMesh()  = default;
    ~VulkanMesh() = default;

    explicit VulkanMesh(const Mesh& baseMesh) : Mesh(baseMesh) {}

    VulkanMesh(const VulkanMesh&)            = delete;
    VulkanMesh& operator=(const VulkanMesh&) = delete;

    VulkanMesh(VulkanMesh&&)            noexcept = default;
    VulkanMesh& operator=(VulkanMesh&&) noexcept = default;

    static VulkanMesh makeFullscreenTriangle();

    [[nodiscard]] const VulkanBuffer* getVertexBuffer() const noexcept { return _vertexBuffer; }
    [[nodiscard]] const VulkanBuffer* getIndexBuffer() const noexcept { return _indexBuffer; }

    void setVertexBuffer(const VulkanBuffer* vertexBuffer) noexcept { _vertexBuffer = vertexBuffer; }
    void setIndexBuffer(const VulkanBuffer* indexBuffer) noexcept { _indexBuffer = indexBuffer; }

    [[nodiscard]] bool isBufferless() const noexcept { return _bufferless; }
    void setBufferless(const bool bufferless) noexcept { _bufferless = bufferless; }

    [[nodiscard]] size_t getVertexOffset() const noexcept { return _vertexOffset; }
    [[nodiscard]] size_t getIndexOffset() const noexcept { return _indexOffset; }

    void setVertexOffset(const size_t offset) noexcept { _vertexOffset = offset; }

    void setIndexOffset(const size_t offset) noexcept { _indexOffset = offset; }

private:
    const VulkanBuffer* _vertexBuffer = nullptr;
    const VulkanBuffer* _indexBuffer  = nullptr;

    bool _bufferless = false;

    size_t _vertexOffset = 0;
    size_t _indexOffset  = 0;
};
