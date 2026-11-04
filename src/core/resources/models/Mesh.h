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

    bool operator==(const Mesh& other) const noexcept {
        if (_vertices.empty() && other._vertices.empty()) { return true; }

        return _vertices.size()  == other._vertices.size()
            && _indices.size()   == other._indices.size()
            && _vertices.front() == other._vertices.front()
            && _vertices.back()  == other._vertices.back()
            && _obb              == other._obb;
    }

    // Generates averaged normals for a given range of vertices and indices
    void generateSmoothNormals(std::size_t vertexStart, std::size_t vertexEnd, std::size_t indexStart, std::size_t indexEnd);
    // Generates averaged normals for the entire mesh
    void generateSmoothNormals();

    // Generates tangents for the entire mesh
    void generateTangents();

    void computeOBB(const glm::mat3& hintFrame = glm::mat3(1.0f));

    [[nodiscard]] static Mesh makeOBBLines(const Math::OBB& obb);

    [[nodiscard]]       std::vector<Vertex>& getVertices()       noexcept { return _vertices; }
    [[nodiscard]] const std::vector<Vertex>& getVertices() const noexcept { return _vertices; }

    [[nodiscard]]       std::vector<std::uint32_t>& getIndices()       noexcept { return _indices; }
    [[nodiscard]] const std::vector<std::uint32_t>& getIndices() const noexcept { return _indices; }

    [[nodiscard]] std::size_t getVerticesByteSize() const { return sizeof(Vertex) * _vertices.size(); }
    [[nodiscard]] std::size_t getIndicesByteSize() const { return sizeof(std::uint32_t) * _indices.size(); }

    [[nodiscard]] const Math::OBB& getOBB() const noexcept { return _obb; }

    [[nodiscard]] const Material& getMaterial() const noexcept { return _material; }

    void addVertex(const Vertex& vertex) { _vertices.push_back(vertex); }
    void addIndex(const std::uint32_t index) { _indices.push_back(index); }

    void loadData(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices) noexcept {
        _vertices = vertices;
        _indices  = indices;
    }

    void setOBB(const Math::OBB& obb) noexcept { _obb = obb; }

    void setMaterial(const Material& material) noexcept { _material = material; }

protected:
    std::vector<Vertex>        _vertices{};
    std::vector<std::uint32_t> _indices{};

    Math::OBB _obb{};

    Material _material{};
};

struct MeshHash {
    std::size_t operator()(const Mesh& mesh) const noexcept {
        std::size_t hash = 0;

        HashUtils::combine(hash, mesh.getVertices().size());
        HashUtils::combine(hash, mesh.getIndices().size());

        // Hash first and last vertex as a cheap fingerprint
        if (!mesh.getVertices().empty()) {
            HashUtils::combine(hash, mesh.getVertices().front());
            HashUtils::combine(hash, mesh.getVertices().back());
        }

        return hash;
    }
};
