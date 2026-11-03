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

    [[nodiscard]] std::int32_t getVertexOffset() const noexcept { return _vertexOffset; }
    [[nodiscard]] std::uint32_t getFirstIndex() const noexcept { return _firstIndex; }

    void setVertexOffset(const std::int32_t vertexOffset) noexcept { _vertexOffset = vertexOffset; }
    void setFirstIndex(const std::uint32_t firstIndex) noexcept { _firstIndex = firstIndex; }

private:
    std::int32_t  _vertexOffset = 0;
    std::uint32_t _firstIndex   = 0;
};
