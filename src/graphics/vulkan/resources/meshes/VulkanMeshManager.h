#pragma once
#ifndef NOBLEENGINE_VULKANMESHMANAGER_H
#define NOBLEENGINE_VULKANMESHMANAGER_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/core/VulkanCommandManager.h"
#include "graphics/vulkan/core/memory/VulkanBuffer.h"
#include "VulkanMesh.h"

#include <vector>

class VulkanMeshManager {
public:
    using MeshesVector = std::vector<std::unique_ptr<VulkanMesh>>;

    VulkanMeshManager()  = default;
    ~VulkanMeshManager() = default;

    VulkanMeshManager(const VulkanMeshManager&)            = delete;
    VulkanMeshManager& operator=(const VulkanMeshManager&) = delete;

    VulkanMeshManager(VulkanMeshManager&&)            = delete;
    VulkanMeshManager& operator=(VulkanMeshManager&&) = delete;

    [[nodiscard]] bool create(
        const VulkanDevice&         device,
        const VulkanCommandManager& commandManager,
        std::string&                errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] VulkanMesh* allocateMesh(const Mesh& meshData);

    [[nodiscard]] bool fillBuffers(std::string& errorMessage);

    [[nodiscard]] const VulkanBuffer& getVertexBuffer() const noexcept { return _vertexBuffer; }
    [[nodiscard]] const VulkanBuffer& getIndexBuffer()  const noexcept { return _indexBuffer; }

private:
    void queryVertexBufferSize();
    void queryIndexBufferSize();

    void copyMeshData(void* stagingData);

    bool createMeshStagingBuffer(std::string& errorMessage);

    bool createVertexBuffer(std::string& errorMessage);
    bool createIndexBuffer(std::string& errorMessage);

    const VulkanDevice*         _device         = nullptr;
    const VulkanCommandManager* _commandManager = nullptr;

    VulkanBuffer _stagingBuffer{};
    VulkanBuffer _vertexBuffer{};
    VulkanBuffer _indexBuffer{};

    MeshesVector _meshes{};

    size_t _currentVertexOffset = 0;
    size_t _currentIndexOffset  = 0;

    size_t _vertexBufferSize = 0;
    size_t _indexBufferSize  = 0;
};

#endif // NOBLEENGINE_VULKANMESHMANAGER_H
