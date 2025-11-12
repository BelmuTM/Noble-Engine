#include "Mesh.h"

void Mesh::generateSmoothNormals(
    const size_t vertexStart,
    const size_t vertexEnd,
    const size_t indexStart,
    const size_t indexEnd
) {
    for (size_t i = indexStart; i < indexEnd; i += 3) {
        const uint32_t i0 = _indices[i + 0];
        const uint32_t i1 = _indices[i + 1];
        const uint32_t i2 = _indices[i + 2];

        if (i0 < vertexStart || i0 >= vertexEnd) continue;
        if (i1 < vertexStart || i1 >= vertexEnd) continue;
        if (i2 < vertexStart || i2 >= vertexEnd) continue;

        const glm::vec3 edge1 = _vertices[i1].position - _vertices[i0].position;
        const glm::vec3 edge2 = _vertices[i2].position - _vertices[i0].position;

        const glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

        _vertices[i0].normal += faceNormal;
        _vertices[i1].normal += faceNormal;
        _vertices[i2].normal += faceNormal;
    }

    for (uint32_t i = vertexStart; i < vertexEnd; i++) {
        _vertices[i].normal = glm::normalize(_vertices[i].normal);
    }
}

void Mesh::generateSmoothNormals() {
    generateSmoothNormals(0, _vertices.size(), 0, _indices.size());
}
