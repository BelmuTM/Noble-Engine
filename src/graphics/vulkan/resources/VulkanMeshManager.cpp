#include "VulkanMeshManager.h"

#include "core/debug/ErrorHandling.h"

bool VulkanMeshManager::create(
    const VulkanDevice&            device,
    const VulkanCommandManager&    commandManager,
    const std::vector<VulkanMesh>& meshes,
    std::string&                   errorMessage
) noexcept {
    _device         = &device;
    _commandManager = &commandManager;

    _meshes.assign(meshes.begin(), meshes.end());

    queryVertexBufferSize();
    queryIndexBufferSize();

    TRY(createStagingBuffer(errorMessage));
    TRY(createVertexBuffer(errorMessage));
    TRY(createIndexBuffer(errorMessage));

    _stagingBuffer.destroy();
    return true;
}

void VulkanMeshManager::destroy() noexcept {
    _indexBuffer.destroy();
    _vertexBuffer.destroy();
}

void VulkanMeshManager::queryVertexBufferSize() {
    for (const auto& mesh : _meshes) {
        _vertexBufferSize += mesh.getVerticesByteSize();
    }
}

void VulkanMeshManager::queryIndexBufferSize() {
    for (const auto& mesh : _meshes) {
        _indexBufferSize += mesh.getIndicesByteSize();
    }
}

void VulkanMeshManager::copyMeshData(void* stagingData) {
    _currentIndexOffset = _vertexBufferSize;

    for (auto& mesh : _meshes) {
        const size_t verticesSize = mesh.getVerticesByteSize();
        const size_t indicesSize  = mesh.getIndicesByteSize();

        mesh.setVertexOffset(_currentVertexOffset);
        mesh.setIndexOffset(_currentIndexOffset);

        memcpy(static_cast<char*>(stagingData) + _currentVertexOffset, mesh.getVertices().data(), verticesSize);
        memcpy(static_cast<char*>(stagingData) + _currentIndexOffset, mesh.getIndices().data(), indicesSize);

        _currentVertexOffset += verticesSize;
        _currentIndexOffset += indicesSize;
    }
}

bool VulkanMeshManager::createStagingBuffer(std::string& errorMessage) {
    const vk::DeviceSize stagingBufferSize = _vertexBufferSize + _indexBufferSize;

    TRY(_stagingBuffer.create(
        stagingBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        _device,
        errorMessage
    ));

    // Mapping GPU allocated memory to CPU memory
    void* stagingData = _stagingBuffer.mapMemory(errorMessage);
    if (!stagingData) return false;

    copyMeshData(stagingData);

    _stagingBuffer.unmapMemory();
    return true;
}

bool VulkanMeshManager::createVertexBuffer(std::string& errorMessage) {
    TRY(_vertexBuffer.create(
        _vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        VMA_MEMORY_USAGE_GPU_ONLY,
        _device,
        errorMessage
    ));

    TRY(_vertexBuffer.copyFrom(_stagingBuffer, _commandManager, errorMessage));
    return true;
}

bool VulkanMeshManager::createIndexBuffer(std::string& errorMessage) {
    TRY(_indexBuffer.create(
        _indexBufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        VMA_MEMORY_USAGE_GPU_ONLY,
        _device,
        errorMessage
    ));

    TRY(_indexBuffer.copyFrom(_stagingBuffer, _commandManager, errorMessage, _indexBufferSize, _vertexBufferSize, 0));
    return true;
}
