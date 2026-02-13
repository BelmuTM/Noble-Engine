#pragma once
#ifndef NOBLEENGINE_MATH_H
#define NOBLEENGINE_MATH_H

#include <glm/glm.hpp>

class Math {
public:
    struct AABB {
        glm::vec3 minBound = { FLT_MAX,  FLT_MAX,  FLT_MAX};
        glm::vec3 maxBound = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

        static AABB transform(const AABB& aabb, const glm::mat4& m);
    };

    struct Plane {
        glm::vec3 normal;
        float d;
    };

    [[nodiscard]] static bool within(float value, float minBound, float maxBound);
};

#endif // NOBLEENGINE_MATH_H
