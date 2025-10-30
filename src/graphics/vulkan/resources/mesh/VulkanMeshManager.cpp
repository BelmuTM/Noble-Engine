#include "VulkanMeshManager.h"

#include "core/debug/ErrorHandling.h"
#include "core/debug/Logger.h"
#include "core/ResourceManager.h"

#include "graphics/vulkan/resources/mesh/TinyObjLoaderUsage.h"

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
        _currentIndexOffset  += indicesSize;
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

std::optional<VulkanMesh> VulkanMeshManager::loadModel(const std::string& path, std::string& errorMessage) {
    VulkanMesh model;

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    std::vector<Vertex>   vertices{};
    std::vector<uint32_t> indices{};

    tinyobj::attrib_t attributes;

    std::vector<tinyobj::shape_t>    shapes;
    std::vector<tinyobj::material_t> materials;

    const std::string& fullPath = modelFilesPath + path;

    if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &errorMessage, fullPath.c_str())) {
        return std::nullopt;
    }

    Logger::info("vertices: " + std::to_string(attributes.vertices.size())
           + ", normals: " + std::to_string(attributes.normals.size())
           + ", texcoords: " + std::to_string(attributes.texcoords.size())
           + ", shapes: " + std::to_string(shapes.size()));

    for (const auto& [name, mesh] : shapes) {
        for (const auto& [vertex_index, normal_index, texcoord_index] : mesh.indices) {
            Vertex vertex{};

            vertex.position = {
                attributes.vertices[3 * vertex_index + 0],
                attributes.vertices[3 * vertex_index + 1],
                attributes.vertices[3 * vertex_index + 2]
            };

            if (!attributes.normals.empty() && normal_index >= 0) {
                vertex.normal = {
                    attributes.normals[3 * normal_index + 0],
                    attributes.normals[3 * normal_index + 1],
                    attributes.normals[3 * normal_index + 2]
                };
            }

            if (!attributes.texcoords.empty() && texcoord_index >= 0) {
                vertex.textureCoords = {
                    attributes.texcoords[2 * texcoord_index + 0],
                    1.0f - attributes.texcoords[2 * texcoord_index + 1]
                };
            }

            vertex.color = {1.0f, 1.0f, 1.0f};

            if (!uniqueVertices.contains(vertex)) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    model.loadData(vertices, indices);
    return std::make_optional<VulkanMesh>(model);
}
