#include "Math.h"

Math::AABB Math::AABB::transform(const AABB& aabb, const glm::mat4& m) {
    const glm::vec3 corners[8] = {
        {aabb.minBound.x, aabb.minBound.y, aabb.minBound.z},
        {aabb.maxBound.x, aabb.minBound.y, aabb.minBound.z},
        {aabb.minBound.x, aabb.maxBound.y, aabb.minBound.z},
        {aabb.maxBound.x, aabb.maxBound.y, aabb.minBound.z},
        {aabb.minBound.x, aabb.minBound.y, aabb.maxBound.z},
        {aabb.maxBound.x, aabb.minBound.y, aabb.maxBound.z},
        {aabb.minBound.x, aabb.maxBound.y, aabb.maxBound.z},
        {aabb.maxBound.x, aabb.maxBound.y, aabb.maxBound.z}
    };

    AABB newAABB{};
    for (const auto& corner : corners) {
        glm::vec4 transformedCorner = m * glm::vec4(corner, 1.0f);

        newAABB.minBound = glm::min(newAABB.minBound, glm::vec3(transformedCorner));
        newAABB.maxBound = glm::max(newAABB.maxBound, glm::vec3(transformedCorner));
    }

    return newAABB;
}

bool Math::within(const float value, const float minBound, const float maxBound) {
    return value >= minBound && value <= maxBound;
}
