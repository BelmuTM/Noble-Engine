#pragma once
#ifndef NOBLEENGINE_MESH_H
#define NOBLEENGINE_MESH_H

#include "core/resources/Vertex.h"
#include "Material.h"

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
    void generateSmoothNormals(size_t vertexStart, size_t vertexEnd, size_t indexStart, size_t indexEnd);
    // Generates averaged normals for the entire mesh
    void generateSmoothNormals();

    [[nodiscard]] const std::vector<Vertex>& getVertices() const noexcept { return _vertices; }
    [[nodiscard]] const std::vector<uint32_t>& getIndices() const noexcept { return _indices; }

    [[nodiscard]] size_t getVerticesByteSize() const { return sizeof(Vertex) * _vertices.size(); }
    [[nodiscard]] size_t getIndicesByteSize() const { return sizeof(uint32_t) * _indices.size(); }

    [[nodiscard]] Material getMaterial() const noexcept { return _material; }

    void addVertex(const Vertex& vertex) { _vertices.push_back(vertex); }
    void addIndex(const uint32_t index) { _indices.push_back(index); }

    void loadData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) noexcept {
        _vertices = vertices;
        _indices  = indices;
    }

    void setMaterial(const Material& material) noexcept { _material = material; }

protected:
    std::vector<Vertex>   _vertices{};
    std::vector<uint32_t> _indices{};

    Material _material{};
};

#endif // NOBLEENGINE_MESH_H
