#pragma once
#ifndef NOBLEENGINE_VERTEX_H
#define NOBLEENGINE_VERTEX_H

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct alignas(16) Vertex {
    glm::vec3 position      = {0.0f, 0.0f, 0.0f};
    glm::vec3 normal        = {0.0f, 0.0f, 0.0f};
    glm::vec4 tangent       = {0.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 color         = {1.0f, 1.0f, 1.0f};
    glm::vec2 textureCoords = {0.0f, 0.0f};

    bool operator==(const Vertex& other) const = default;
};

template<> struct std::hash<Vertex> {
    size_t operator()(Vertex const& v) const noexcept {
        static constexpr size_t goldenRatio = 0x9e3779b9;

        size_t seed  = std::hash<glm::vec3>()(v.position);
               seed ^= std::hash<glm::vec3>()(v.normal)        + goldenRatio + (seed << 6) + (seed >> 2);
               seed ^= std::hash<glm::vec3>()(v.tangent)       + goldenRatio + (seed << 6) + (seed >> 2);
               seed ^= std::hash<glm::vec3>()(v.color)         + goldenRatio + (seed << 6) + (seed >> 2);
               seed ^= std::hash<glm::vec2>()(v.textureCoords) + goldenRatio + (seed << 6) + (seed >> 2);
        return seed;
    }
};

#endif // NOBLEENGINE_VERTEX_H
