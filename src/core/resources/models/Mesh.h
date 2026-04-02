#pragma once

#include "common/Math.h"

#include "Material.h"
#include "Vertex.h"

#include <vector>

class Mesh {
public:
    Mesh()  = default;
    ~Mesh() = default;

    Mesh(const Mesh&)            noexcept = default;
    Mesh& operator=(const Mesh&) noexcept = default;

    Mesh(Mesh&&)            noexcept = default;
    Mesh& operator=(Mesh&&) noexcept = default;

    // Generates averaged normals for a given range of vertices and indices
    void generateSmoothNormals(std::size_t vertexStart, std::size_t vertexEnd, std::size_t indexStart, std::size_t indexEnd);
    // Generates averaged normals for the entire mesh
    void generateSmoothNormals();

    // Generates tangents for the entire mesh
    void generateTangents();

    [[nodiscard]]       std::vector<Vertex>& getVertices()       noexcept { return _vertices; }
    [[nodiscard]] const std::vector<Vertex>& getVertices() const noexcept { return _vertices; }

    [[nodiscard]]       std::vector<std::uint32_t>& getIndices()       noexcept { return _indices; }
    [[nodiscard]] const std::vector<std::uint32_t>& getIndices() const noexcept { return _indices; }

    [[nodiscard]] std::size_t getVerticesByteSize() const { return sizeof(Vertex) * _vertices.size(); }
    [[nodiscard]] std::size_t getIndicesByteSize() const { return sizeof(std::uint32_t) * _indices.size(); }

    [[nodiscard]] const Math::AABB& getAABB() const noexcept { return _aabb; }

    [[nodiscard]] const Material& getMaterial() const noexcept { return _material; }

    void addVertex(const Vertex& vertex) { _vertices.push_back(vertex); }
    void addIndex(const std::uint32_t index) { _indices.push_back(index); }

    void loadData(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices) noexcept {
        _vertices = vertices;
        _indices  = indices;
    }

    void setAABB(const Math::AABB& aabb) noexcept { _aabb = aabb; }

    void setMaterial(const Material& material) noexcept { _material = material; }

protected:
    std::vector<Vertex>        _vertices{};
    std::vector<std::uint32_t> _indices{};

    Math::AABB _aabb{};

    Material _material{};
};
