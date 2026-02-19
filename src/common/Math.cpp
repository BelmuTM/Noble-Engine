#include "Math.h"

Math::AABB Math::AABB::transform(const glm::mat4& m) const {
    AABB newAABB{};
    for (const auto& corner : getCorners()) {
        glm::vec4 transformedCorner = m * glm::vec4(corner, 1.0f);

        newAABB.minBound = glm::min(newAABB.minBound, glm::vec3(transformedCorner));
        newAABB.maxBound = glm::max(newAABB.maxBound, glm::vec3(transformedCorner));
    }

    return newAABB;
}

std::array<glm::vec3, 8> Math::AABB::getCorners() const {
    return {
        glm::vec3(minBound.x, minBound.y, minBound.z), // 0: left-bottom-back
        glm::vec3(maxBound.x, minBound.y, minBound.z), // 1: right-bottom-back
        glm::vec3(maxBound.x, maxBound.y, minBound.z), // 2: right-top-back
        glm::vec3(minBound.x, maxBound.y, minBound.z), // 3: left-top-back
        glm::vec3(minBound.x, minBound.y, maxBound.z), // 4: left-bottom-front
        glm::vec3(maxBound.x, minBound.y, maxBound.z), // 5: right-bottom-front
        glm::vec3(maxBound.x, maxBound.y, maxBound.z), // 6: right-top-front
        glm::vec3(minBound.x, maxBound.y, maxBound.z)  // 7: left-top-front
    };
}

std::array<uint32_t, 24> Math::AABB::getLineIndices(const uint32_t startIndex) {
    return {
        startIndex + 0, startIndex + 1,  // back-bottom
        startIndex + 1, startIndex + 2,  // back-right
        startIndex + 2, startIndex + 3,  // back-top
        startIndex + 3, startIndex + 0,  // back-left

        startIndex + 4, startIndex + 5,  // front-bottom
        startIndex + 5, startIndex + 6,  // front-right
        startIndex + 6, startIndex + 7,  // front-top
        startIndex + 7, startIndex + 4,  // front-left

        startIndex + 0, startIndex + 4,  // left-bottom edge
        startIndex + 1, startIndex + 5,  // right-bottom edge
        startIndex + 2, startIndex + 6,  // right-top edge
        startIndex + 3, startIndex + 7   // left-top edge
    };
}

bool Math::within(const float value, const float minBound, const float maxBound) {
    return value >= minBound && value <= maxBound;
}
