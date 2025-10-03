#pragma once
#ifndef NOBLEENGINE_VULKANMESH_H
#define NOBLEENGINE_VULKANMESH_H

#include "graphics/vulkan/common/VulkanHeader.h"
#include "graphics/vulkan/core/memory/VulkanBuffer.h"

#include <glm/glm.hpp>

struct Vertex {
    glm::vec2 position;
    glm::vec3 color;

    static vk::VertexInputBindingDescription getBindingDescription() {
        return {0, sizeof(Vertex), vk::VertexInputRate::eVertex};
    }

    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
        return {
            vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32Sfloat   , offsetof(Vertex, position)},
            vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color   )}
        };
    }
};

class VulkanMesh {
public:
    VulkanMesh()  = default;
    ~VulkanMesh() = default;

    VulkanMesh(const VulkanMesh&) noexcept;
    VulkanMesh& operator=(const VulkanMesh&) noexcept;

    VulkanMesh(VulkanMesh&&) noexcept;
    VulkanMesh& operator=(VulkanMesh&&) noexcept;

    [[nodiscard]] bool create(
        const VulkanDevice& device,
        std::string&        errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] const std::vector<Vertex> getVertices() const { return vertices; }
    [[nodiscard]] const std::vector<uint16_t> getIndices() const { return indices; }

    [[nodiscard]] const size_t getVerticesByteSize() const { return sizeof(Vertex) * vertices.size(); }
    [[nodiscard]] const size_t getIndicesByteSize() const { return sizeof(uint16_t) * indices.size(); }

    [[nodiscard]] const size_t getVertexOffset() const { return _vertexOffset; }
    [[nodiscard]] const size_t getIndexOffset() const { return _indexOffset; }

    void setVertexOffset(const size_t offset) {
        _vertexOffset = offset;
    }

    void setIndexOffset(const size_t offset) {
        _indexOffset = offset;
    }

private:
    const VulkanDevice* _device = nullptr;

    size_t _vertexOffset = 0;
    size_t _indexOffset  = 0;

    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
        {{0.0f, -1.0f}, {1.0f, 1.0f, 0.0f}}
    };

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0, 0, 4, 1
    };
};

#endif // NOBLEENGINE_VULKANMESH_H
