#pragma once
#ifndef NOBLEENGINE_MESH_H
#define NOBLEENGINE_MESH_H

#include "Vertex.h"
#include "Material.h"

#include "core/common/tinygltfUsage.h"
#include "core/common/tinyobjloaderUsage.h"

#include <string>
#include <vector>

class Mesh {
public:
    Mesh()  = default;
    ~Mesh() = default;

    Mesh(const Mesh&)            noexcept = default;
    Mesh& operator=(const Mesh&) noexcept = default;

    Mesh(Mesh&&)            noexcept = default;
    Mesh& operator=(Mesh&&) noexcept = default;

    [[nodiscard]] const std::vector<Vertex>& getVertices() const noexcept { return _vertices; }
    [[nodiscard]] const std::vector<uint32_t>& getIndices() const noexcept { return _indices; }

    [[nodiscard]] size_t getVerticesByteSize() const { return sizeof(Vertex) * _vertices.size(); }
    [[nodiscard]] size_t getIndicesByteSize() const { return sizeof(uint32_t) * _indices.size(); }

    [[nodiscard]] Material getMaterial() const noexcept { return _material; }

    void loadData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) noexcept {
        _vertices = vertices;
        _indices  = indices;
    }

    void loadMaterialObj(const tinyobj::material_t& material);
    void loadMaterialGLTF(
        const tinygltf::Material&             material,
        const std::vector<tinygltf::Texture>& textures,
        const std::vector<tinygltf::Image>&   images
    );

    [[nodiscard]] bool loadObj(const std::string& path, std::string& errorMessage);
    [[nodiscard]] bool loadGLTF(const std::string& path, std::string& errorMessage);

    [[nodiscard]] bool load(const std::string& path, std::string& errorMessage);

private:
    std::string _name = "Undefined_Mesh";

    std::vector<Vertex>   _vertices{};
    std::vector<uint32_t> _indices{};

    Material _material;

    void generateSmoothNormals(size_t vertexStart, size_t vertexEnd, size_t indexStart, size_t indexEnd);
};

#endif // NOBLEENGINE_MESH_H
