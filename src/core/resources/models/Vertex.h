#pragma once

#include "common/HashUtils.h"

#include <glm/glm.hpp>

struct alignas(16) Vertex {
    glm::vec3 position      = {0.0f, 0.0f, 0.0f};
    glm::vec3 normal        = {0.0f, 0.0f, 0.0f};
    glm::vec4 tangent       = {0.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 color         = {1.0f, 1.0f, 1.0f};
    glm::vec2 textureCoords = {0.0f, 0.0f};

    bool operator==(const Vertex& other) const = default;
};

template<>
struct std::hash<Vertex> {
    std::size_t operator()(Vertex const& v) const noexcept {
        std::size_t hash = 0;

        HashUtils::combine(hash, v.position);
        HashUtils::combine(hash, v.normal);
        HashUtils::combine(hash, v.tangent);
        HashUtils::combine(hash, v.color);
        HashUtils::combine(hash, v.textureCoords);

        return hash;
    }
};
