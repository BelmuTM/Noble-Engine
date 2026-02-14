#include "VulkanMeshManager.h"

#include "core/debug/ErrorHandling.h"
#include "core/debug/Logger.h"

bool VulkanMeshManager::create(
    const VulkanDevice&         device,
    const VulkanCommandManager& commandManager,
    std::string&                errorMessage
) noexcept {
    _device         = &device;
    _commandManager = &commandManager;

    return true;
}

void VulkanMeshManager::destroy() noexcept {
    _indexBuffer.destroy();
    _vertexBuffer.destroy();

    _device         = nullptr;
    _commandManager = nullptr;
}

VulkanMesh* VulkanMeshManager::allocateMesh(const Mesh& meshData) {
    _meshes.push_back(std::make_unique<VulkanMesh>(meshData));
    return _meshes.back().get();
}

bool VulkanMeshManager::fillBuffers(std::string& errorMessage) {
    queryVertexBufferSize();
    queryIndexBufferSize();

    TRY(createMeshStagingBuffer(errorMessage));

    TRY(createVertexBuffer(errorMessage));
    TRY(createIndexBuffer(errorMessage));

    _stagingBuffer.destroy();

    return true;
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

void VulkanMeshManager::copyMeshData(void* stagingData) {
    _currentIndexOffset = _vertexBufferSize;

    for (const auto& mesh : _meshes) {
        if (mesh->isBufferless()) continue;

        const size_t verticesSize = mesh->getVerticesByteSize();
        const size_t indicesSize  = mesh->getIndicesByteSize();

        mesh->setVertexOffset(_currentVertexOffset);
        mesh->setIndexOffset(_currentIndexOffset - _vertexBufferSize);

        std::memcpy(static_cast<char*>(stagingData) + _currentVertexOffset, mesh->getVertices().data(), verticesSize);
        std::memcpy(static_cast<char*>(stagingData) + _currentIndexOffset , mesh->getIndices().data() , indicesSize);

        _currentVertexOffset += verticesSize;
        _currentIndexOffset  += indicesSize;
    }
}

bool VulkanMeshManager::createMeshStagingBuffer(std::string& errorMessage) {
    const vk::DeviceSize stagingBufferSize = _vertexBufferSize + _indexBufferSize;

    TRY(_stagingBuffer.create(
        stagingBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        _device,
        errorMessage
    ));

    void* stagingData = _stagingBuffer.mapMemory(errorMessage);

    if (!stagingData) {
        errorMessage = "Failed to create Vulkan mesh staging buffer: mapped memory pointer is null.";
        return false;
    }

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
