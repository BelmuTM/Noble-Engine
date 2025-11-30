#include "Mesh.h"

#include <cmath>

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

void Mesh::generateTangents(
    const size_t vertexStart,
    const size_t vertexEnd,
    const size_t indexStart,
    const size_t indexEnd
) {
    std::vector bitangentAccum(_vertices.size(), glm::vec3(0.0f));

    for (size_t i = indexStart; i < indexEnd; i += 3) {
        uint32_t i0 = _indices[i + 0];
        uint32_t i1 = _indices[i + 1];
        uint32_t i2 = _indices[i + 2];

        if (i0 < vertexStart || i0 >= vertexEnd) continue;
        if (i1 < vertexStart || i1 >= vertexEnd) continue;
        if (i2 < vertexStart || i2 >= vertexEnd) continue;

        auto& v0 = _vertices[i0];
        auto& v1 = _vertices[i1];
        auto& v2 = _vertices[i2];

        const glm::vec3 edge1 = v1.position - v0.position;
        const glm::vec3 edge2 = v2.position - v0.position;

        const glm::vec2 dUV1 = v1.textureCoords - v0.textureCoords;
        const glm::vec2 dUV2 = v2.textureCoords - v0.textureCoords;

        float f = dUV1.x * dUV2.y - dUV1.y * dUV2.x;
              f = std::fabs(f) > 1e-8f ? 1.0f / f : 1.0f;

        glm::vec3 tangent   = f * (edge1 * dUV2.y - edge2 * dUV1.y);
        glm::vec3 bitangent = f * (edge2 * dUV1.x - edge1 * dUV2.x);

        _vertices[i0].tangent += glm::vec4(tangent, 0.0f);
        _vertices[i1].tangent += glm::vec4(tangent, 0.0f);
        _vertices[i2].tangent += glm::vec4(tangent, 0.0f);

        bitangentAccum[i0] += bitangent;
        bitangentAccum[i1] += bitangent;
        bitangentAccum[i2] += bitangent;
    }

    // Normalization + handedness
    for (size_t i = vertexStart; i < vertexEnd; i++) {
        glm::vec3 N = _vertices[i].normal;
        glm::vec3 T = glm::xyz(_vertices[i].tangent);
        glm::vec3 B = bitangentAccum[i];

        // Orthonormalize T
        T = glm::normalize(T - N * glm::dot(N, T));

        // Handedness
        float w = glm::dot(glm::cross(N, T), B) < 0.0f ? -1.0f : 1.0f;

        _vertices[i].tangent = glm::vec4(T, w);
    }
}

void Mesh::generateTangents() {
    generateTangents(0, _vertices.size(), 0, _indices.size());
}
