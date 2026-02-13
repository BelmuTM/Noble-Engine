#include "FrustumCuller.h"

std::array<Math::Plane, 6> FrustumCuller::getFrustumPlanes(const glm::mat4& viewProjectionMatrix) {
    std::array<Math::Plane, 6> planes{};
    auto& m = viewProjectionMatrix;

    // Left
    planes[0].normal = glm::vec3(m[0][3] + m[0][0], m[1][3] + m[1][0], m[2][3] + m[2][0]);
    planes[0].d = m[3][3] + m[3][0];

    // Right
    planes[1].normal = glm::vec3(m[0][3] - m[0][0], m[1][3] - m[1][0], m[2][3] - m[2][0]);
    planes[1].d = m[3][3] - m[3][0];

    // Bottom
    planes[2].normal = glm::vec3(m[0][3] + m[0][2], m[1][3] + m[1][2], m[2][3] + m[2][2]);
    planes[2].d = m[3][3] + m[3][2];

    // Top
    planes[3].normal = glm::vec3(m[0][3] - m[0][2], m[1][3] - m[1][2], m[2][3] - m[2][2]);
    planes[3].d = m[3][3] - m[3][2];

    // Near
    planes[4].normal = glm::vec3(m[0][3] + m[0][1], m[1][3] + m[1][1], m[2][3] + m[2][1]);
    planes[4].d = m[3][3] + m[3][1];

    // Far
    planes[5].normal = glm::vec3(m[0][3] - m[0][1], m[1][3] - m[1][1], m[2][3] - m[2][1]);
    planes[5].d = m[3][3] - m[3][1];

    // Normalize
    for (auto& [normal, d] : planes) {
        const float len = glm::length(normal);
        normal /= len;
        d      /= len;
    }

    return planes;
}


bool FrustumCuller::testVisibility(const Math::AABB& aabb, const std::array<Math::Plane, 6>& frustumPlanes) {
    for (const auto& [normal, d] : frustumPlanes) {
        glm::vec3 p = aabb.maxBound;
        if (normal.x < 0.0) p.x = aabb.minBound.x;
        if (normal.y < 0.0) p.y = aabb.minBound.y;
        if (normal.z < 0.0) p.z = aabb.minBound.z;

        if (glm::dot(normal, p) + d < 0.001f) {
            return false;
        }
    }

    return true;
}
