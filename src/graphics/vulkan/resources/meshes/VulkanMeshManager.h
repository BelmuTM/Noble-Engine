#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "VulkanMesh.h"

#include "graphics/vulkan/core/VulkanCommandManager.h"
#include "graphics/vulkan/core/memory/VulkanBuffer.h"

#include <vector>

class VulkanMeshManager {
public:
    VulkanMeshManager()  = default;
    ~VulkanMeshManager() = default;

    VulkanMeshManager(const VulkanMeshManager&)            = delete;
    VulkanMeshManager& operator=(const VulkanMeshManager&) = delete;

    VulkanMeshManager(VulkanMeshManager&&)            = delete;
    VulkanMeshManager& operator=(VulkanMeshManager&&) = delete;

    [[nodiscard]] Expected<void> create(
        const VulkanDevice& device, const VulkanCommandManager& commandManager
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] VulkanMesh* allocateMesh(const Mesh& meshData);

    [[nodiscard]] Expected<void> fillBuffers();

    [[nodiscard]] const VulkanBuffer& getVertexBuffer() const noexcept { return _vertexBuffer; }
    [[nodiscard]] const VulkanBuffer& getIndexBuffer()  const noexcept { return _indexBuffer; }

private:
    void queryVertexBufferSize();
    void queryIndexBufferSize();

    void uploadMeshData(const VulkanBuffer& buffer);

    void assignBuffersToMeshes() const;

    Expected<void> createMeshStagingBuffer();

    Expected<void> createVertexBuffer();
    Expected<void> createIndexBuffer();

    const VulkanDevice*         _device         = nullptr;
    const VulkanCommandManager* _commandManager = nullptr;

    VulkanBuffer _stagingBuffer{};
    VulkanBuffer _vertexBuffer{};
    VulkanBuffer _indexBuffer{};

    std::vector<std::unique_ptr<VulkanMesh>> _meshes{};

    std::size_t _currentVertexOffset = 0;
    std::size_t _currentIndexOffset  = 0;

    std::size_t _vertexBufferSize = 0;
    std::size_t _indexBufferSize  = 0;
};
