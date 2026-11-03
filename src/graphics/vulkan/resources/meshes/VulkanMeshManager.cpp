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

    if (const auto cachedMesh = _meshCache.find(meshData); cachedMesh != _meshCache.end()) {
        return cachedMesh->second;
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

    _stagingBuffer.destroy();

    return {};
}

void VulkanMeshManager::queryVertexBufferSize() {
    for (const auto& mesh : _meshes) {
        _vertexBufferSize += mesh->getVerticesByteSize();
    }
}

void VulkanMeshManager::queryIndexBufferSize() {
    for (const auto& mesh : _meshes) {
        _indexBufferSize += mesh->getIndicesByteSize();
    }
}

void VulkanMeshManager::uploadMeshData(const VulkanBuffer& buffer) const {

    std::size_t vertexByteOffset = 0;
    std::size_t indexByteOffset  = _vertexBufferSize;

    std::int32_t  vertexOffset = 0;
    std::uint32_t firstIndex   = 0;

    for (const auto& mesh : _meshes) {

        const std::size_t verticesSize = mesh->getVerticesByteSize();
        const std::size_t indicesSize  = mesh->getIndicesByteSize();

        mesh->setVertexOffset(vertexOffset);
        mesh->setFirstIndex(firstIndex);

        buffer.updateMemory(mesh->getVertices().data(), verticesSize, vertexByteOffset);
        buffer.updateMemory(mesh->getIndices().data() , indicesSize , indexByteOffset);

        vertexByteOffset += verticesSize;
        indexByteOffset  += indicesSize;

        vertexOffset += static_cast<std::int32_t>(mesh->getVertices().size());
        firstIndex   += static_cast<std::uint32_t>(mesh->getIndices().size());
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
