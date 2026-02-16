#pragma once

#include "core/resources/models/Mesh.h"

class FrustumCuller {
public:
    FrustumCuller()  = default;
    ~FrustumCuller() = default;

    [[nodiscard]] static std::array<Math::Plane, 6> getFrustumPlanes(const glm::mat4& viewProjectionMatrix);

    [[nodiscard]] static bool testVisibility(const Math::AABB& aabb, const std::array<Math::Plane, 6>& frustumPlanes);
};
