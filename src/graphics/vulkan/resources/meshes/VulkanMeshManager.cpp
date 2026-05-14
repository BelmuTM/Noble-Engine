#include "VulkanMeshManager.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

Expected<void> VulkanMeshManager::create(
    const VulkanDevice& device, const VulkanCommandManager& commandManager
) noexcept {
    _device         = &device;
    _commandManager = &commandManager;

    return {};
}

void VulkanMeshManager::destroy() noexcept {
    _indexBuffer.destroy();
    _vertexBuffer.destroy();

    _device         = nullptr;
    _commandManager = nullptr;
}

VulkanMesh* VulkanMeshManager::allocateMesh(const Mesh& meshData) {
    if (const auto it = _meshCache.find(meshData); it != _meshCache.end()) {
        return it->second;
    }

    _meshes.push_back(std::make_unique<VulkanMesh>(meshData));
    VulkanMesh* meshPtr = _meshes.back().get();

    _meshCache[meshData] = meshPtr;

    return meshPtr;
}

Expected<void> VulkanMeshManager::fillBuffers() {
    queryVertexBufferSize();
    queryIndexBufferSize();

    TRY(createMeshStagingBuffer());

    TRY(createVertexBuffer());
    TRY(createIndexBuffer());

    assignBuffersToMeshes();

    _stagingBuffer.destroy();

    return {};
}

void VulkanMeshManager::queryVertexBufferSize() {
    for (const auto& mesh : _meshes) {
        if (!mesh || mesh->isBufferless()) continue;
        _vertexBufferSize += mesh->getVerticesByteSize();
    }
}

void VulkanMeshManager::queryIndexBufferSize() {
    for (const auto& mesh : _meshes) {
        if (!mesh || mesh->isBufferless()) continue;
        _indexBufferSize += mesh->getIndicesByteSize();
    }
}

void VulkanMeshManager::uploadMeshData(const VulkanBuffer& buffer) {
    _currentIndexOffset = _vertexBufferSize;

    for (const auto& mesh : _meshes) {
        if (mesh->isBufferless()) continue;

        const std::size_t verticesSize = mesh->getVerticesByteSize();
        const std::size_t indicesSize  = mesh->getIndicesByteSize();

        mesh->setVertexOffset(_currentVertexOffset);
        mesh->setIndexOffset(_currentIndexOffset - _vertexBufferSize);

        buffer.updateMemory(mesh->getVertices().data(), verticesSize, _currentVertexOffset);
        buffer.updateMemory(mesh->getIndices().data() , indicesSize , _currentIndexOffset );

        _currentVertexOffset += verticesSize;
        _currentIndexOffset  += indicesSize;
    }
}

void VulkanMeshManager::assignBuffersToMeshes() const {
    for (const auto& mesh : _meshes) {
        if (!mesh || mesh->isBufferless()) continue;

        mesh->setVertexBuffer(&_vertexBuffer);
        mesh->setIndexBuffer(&_indexBuffer);
    }
}

Expected<void> VulkanMeshManager::createMeshStagingBuffer() {
    const vk::DeviceSize stagingBufferSize = _vertexBufferSize + _indexBufferSize;

    TRY(_stagingBuffer.create(
        stagingBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        _device
    ));

    TRY(_stagingBuffer.mapMemory());

    uploadMeshData(_stagingBuffer);

    _stagingBuffer.unmapMemory();

    return {};
}

Expected<void> VulkanMeshManager::createVertexBuffer() {
    TRY(_vertexBuffer.create(
        _vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        VMA_MEMORY_USAGE_GPU_ONLY,
        _device
    ));

    TRY(_vertexBuffer.copyFrom(_stagingBuffer.handle(), _commandManager));

    return {};
}

Expected<void> VulkanMeshManager::createIndexBuffer() {
    TRY(_indexBuffer.create(
        _indexBufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        VMA_MEMORY_USAGE_GPU_ONLY,
        _device
    ));

    TRY(_indexBuffer.copyFrom(_stagingBuffer.handle(), _commandManager, _indexBufferSize, _vertexBufferSize, 0));

    return {};
}
