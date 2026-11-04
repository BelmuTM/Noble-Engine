#include "Math.h"

#include <array>
#include <cfloat>
#include <cstdint>
#include <vector>

Math::AABB Math::AABB::transform(const glm::mat4& m) const noexcept {
    AABB newAABB{};

    for (const auto& corner : getCorners()) {
        glm::vec4 transformedCorner = m * glm::vec4(corner, 1.0f);

        newAABB.minBound = glm::min(newAABB.minBound, glm::vec3(transformedCorner));
        newAABB.maxBound = glm::max(newAABB.maxBound, glm::vec3(transformedCorner));
    }

    return newAABB;
}

std::array<glm::vec3, 8> Math::AABB::getCorners() const noexcept {
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

std::array<std::uint32_t, 24> Math::AABB::getLineIndices(const std::uint32_t startIndex) noexcept {
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

namespace {

glm::dmat3 eigenvectorsSymmetric(glm::dmat3 matrix) {
    // Jacobi method for 3x3 symmetric matrices
    glm::dmat3 eigenVectors(1.0);

    static constexpr int iterationsJacobi = 32;

    for (int i = 0; i < iterationsJacobi; i++) {

        // Largest off-diagonal entry of the matrix, value matrix[x][y]
        double largest = glm::abs(matrix[1][0]);
        int x = 1, y = 0;

        if (glm::abs(matrix[2][0]) > largest) { largest = glm::abs(matrix[2][0]); x = 2; y = 0; }
        if (glm::abs(matrix[2][1]) > largest) { largest = glm::abs(matrix[2][1]); x = 2; y = 1; }
        // Stop when the matrix is diagonal enough
        if (largest < 1e-12) break;

        // cot(2 * angle)
        const double theta = (matrix[x][x] - matrix[y][y]) / (2.0 * matrix[x][y]);

        const double tanTheta = (theta >= 0.0 ? 1.0 : -1.0) / (glm::abs(theta) + glm::sqrt(theta * theta + 1.0));
        const double cosTheta = 1.0 / glm::sqrt(tanTheta * tanTheta + 1.0);
        const double sinTheta = tanTheta * cosTheta;

        glm::dmat3 rotation(1.0);
        rotation[y][y] = cosTheta; rotation[x][x] = cosTheta;
        rotation[x][y] = sinTheta; rotation[y][x] = -sinTheta;

        matrix = glm::transpose(rotation) * matrix * rotation;

        eigenVectors = eigenVectors * rotation;
    }

    return eigenVectors;
}

bool orthonormalize(glm::mat3& matrix) {

    for (int i = 0; i < 3; i++) {
        // Degenerate
        if (glm::length(matrix[i]) < 1e-8f) return false;
    }

    matrix[0]  = glm::normalize(matrix[0]);
    matrix[1] -= matrix[0] * glm::dot(matrix[0], matrix[1]);

    if (glm::length(matrix[1]) < 1e-6f) return false;

    matrix[1] = glm::normalize(matrix[1]);
    matrix[2] = glm::cross(matrix[0], matrix[1]);

    return true;
}

Math::AABB boundsInFrame(const std::span<const glm::vec3> points, const glm::mat3& frame) {
    Math::AABB aabb{};

    const glm::mat3 inverse = glm::transpose(frame);

    for (const glm::vec3& position : points) {
        const glm::vec3 localPosition = inverse * position;

        aabb.minBound = glm::min(aabb.minBound, localPosition);
        aabb.maxBound = glm::max(aabb.maxBound, localPosition);
    }

    return aabb;
}

float surfaceCost(const Math::AABB& aabb) {
    const glm::vec3 e = aabb.maxBound - aabb.minBound;
    return e.x * e.y + e.y * e.z + e.z * e.x;
}

}

Math::OBB Math::fitOBB(const std::span<const glm::vec3> points, const glm::mat3& hintFrame) {

    if (points.empty()) return {};

    std::vector frames{glm::mat3(1.0f)};

    glm::mat3 hint = hintFrame;
    if (orthonormalize(hint)) {
        frames.push_back(hint);
    }

    glm::dvec3 center(0.0);
    for (const glm::vec3& position : points) {
        center += glm::dvec3(position);
    }
    center /= static_cast<double>(points.size());

    glm::dmat3 covariance(0.0);

    for (const glm::vec3& position : points) {
        const glm::dvec3 offsetFromCenter = glm::dvec3(position) - center;

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                covariance[i][j] += offsetFromCenter[j] * offsetFromCenter[i];
            }
        }
    }

    glm::mat3 pca = glm::mat3(eigenvectorsSymmetric(covariance));

    if (orthonormalize(pca)) {
        frames.push_back(pca);
    }

    OBB best{};
    float bestCost = FLT_MAX; // Tightest (minimal) OBB

    for (const glm::mat3& frame : frames) {

        const AABB box = boundsInFrame(points, frame);

        const float cost = surfaceCost(box);

        if (cost < bestCost) {
            bestCost = cost;
            best.local = box;
            best.transformMatrix = glm::mat4(frame);
        }
    }

    return best;
}
