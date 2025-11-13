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
    VulkanMeshManager()  = default;
    ~VulkanMeshManager() = default;

    VulkanMeshManager(const VulkanMeshManager&)            = delete;
    VulkanMeshManager& operator=(const VulkanMeshManager&) = delete;

    VulkanMeshManager(VulkanMeshManager&&)            = delete;
    VulkanMeshManager& operator=(VulkanMeshManager&&) = delete;

    [[nodiscard]] bool create(
        const VulkanDevice&             device,
        const VulkanCommandManager&     commandManager,
        const std::vector<VulkanMesh*>& meshes,
        std::string&                    errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] const VulkanBuffer& getVertexBuffer() const noexcept { return _vertexBuffer; }
    [[nodiscard]] const VulkanBuffer& getIndexBuffer()  const noexcept { return _indexBuffer; }

private:
    const VulkanDevice*         _device         = nullptr;
    const VulkanCommandManager* _commandManager = nullptr;

    VulkanBuffer _stagingBuffer{};
    VulkanBuffer _vertexBuffer{};
    VulkanBuffer _indexBuffer{};

    std::vector<VulkanMesh*> _meshes{};

    size_t _currentVertexOffset = 0;
    size_t _currentIndexOffset  = 0;

    size_t _vertexBufferSize = 0;
    size_t _indexBufferSize  = 0;

    void queryVertexBufferSize();
    void queryIndexBufferSize();

    void copyMeshData(void* stagingData);

    bool createStagingBuffer(std::string& errorMessage);
    bool createVertexBuffer(std::string& errorMessage);
    bool createIndexBuffer(std::string& errorMessage);
};

#endif // NOBLEENGINE_VULKANMESHMANAGER_H
