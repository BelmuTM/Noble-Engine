#pragma once
#ifndef NOBLEENGINE_VULKANMESH_H
#define NOBLEENGINE_VULKANMESH_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct Vertex {
    glm::vec3 position      = {0.0f, 0.0f, 0.0f};
    glm::vec3 normal        = {0.0f, 0.0f, 0.0f};
    glm::vec3 color         = {1.0f, 1.0f, 1.0f};
    glm::vec2 textureCoords = {0.0f, 0.0f};

    static vk::VertexInputBindingDescription getBindingDescription() {
        return {0, sizeof(Vertex), vk::VertexInputRate::eVertex};
    }

    static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions() {
        return {
            vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position     )},
            vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal       )},
            vk::VertexInputAttributeDescription{2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color        )},
            vk::VertexInputAttributeDescription{3, 0, vk::Format::eR32G32Sfloat   , offsetof(Vertex, textureCoords)}
        };
    }

    bool operator==(const Vertex& other) const {
        return position == other.position && color == other.color && textureCoords == other.textureCoords &&
            normal == other.normal;
    }
};

template<> struct std::hash<Vertex> {
    size_t operator()(Vertex const& vertex) const noexcept {
        return (hash<glm::vec3>()(vertex.position) ^
                hash<glm::vec3>()(vertex.color) << 1) >> 1 ^
                hash<glm::vec2>()(vertex.textureCoords) << 1;
    }
};

class VulkanMesh {
public:
    VulkanMesh()  = default;
    ~VulkanMesh() = default;

    VulkanMesh(const VulkanMesh&)            noexcept = default;
    VulkanMesh& operator=(const VulkanMesh&) noexcept = default;

    VulkanMesh(VulkanMesh&&)            = delete;
    VulkanMesh& operator=(VulkanMesh&&) = delete;

    [[nodiscard]] std::vector<Vertex> getVertices() const { return _vertices; }
    [[nodiscard]] std::vector<uint32_t> getIndices() const { return _indices; }

    [[nodiscard]] size_t getVerticesByteSize() const { return sizeof(Vertex) * _vertices.size(); }
    [[nodiscard]] size_t getIndicesByteSize() const { return sizeof(uint32_t) * _indices.size(); }

    [[nodiscard]] size_t getVertexOffset() const { return _vertexOffset; }
    [[nodiscard]] size_t getIndexOffset() const { return _indexOffset; }

    void setVertexOffset(const size_t offset) {
        _vertexOffset = offset;
    }

    void setIndexOffset(const size_t offset) {
        _indexOffset = offset;
    }

    void loadData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
        _vertices = vertices;
        _indices  = indices;
    }

private:
    size_t _vertexOffset = 0;
    size_t _indexOffset  = 0;

    std::vector<Vertex>   _vertices{};
    std::vector<uint32_t> _indices{};
};

#endif // NOBLEENGINE_VULKANMESH_H
