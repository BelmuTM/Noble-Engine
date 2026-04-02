#pragma once

#include <glm/glm.hpp>

class Math {
public:
    static constexpr float EPSILON = 1e-5f;

    struct AABB {
        glm::vec3 minBound = { FLT_MAX,  FLT_MAX,  FLT_MAX};
        glm::vec3 maxBound = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

        [[nodiscard]] AABB transform(const glm::mat4& m) const;

        [[nodiscard]] std::array<glm::vec3, 8> getCorners() const;

        static std::array<std::uint32_t, 24> getLineIndices(std::uint32_t startIndex = 0);

    };

    struct Plane {
        glm::vec3 normal;
        float d;
    };

    [[nodiscard]] static bool within(float value, float minBound, float maxBound);
};
