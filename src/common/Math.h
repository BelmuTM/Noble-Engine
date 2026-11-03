#pragma once

#include <glm/glm.hpp>

class Math {
public:
    static constexpr float EPSILON = 1e-5f;

    struct AABB {
        glm::vec3 minBound = { FLT_MAX,  FLT_MAX,  FLT_MAX};
        glm::vec3 maxBound = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

        [[nodiscard]] AABB transform(const glm::mat4& m) const noexcept;

        [[nodiscard]] std::array<glm::vec3, 8> getCorners() const noexcept;

        [[nodiscard]] static std::array<std::uint32_t, 24> getLineIndices(std::uint32_t startIndex = 0) noexcept;

        bool operator==(const AABB&) const noexcept = default;
    };

    struct OBB {
        AABB local{};

        glm::mat4 transformMatrix = glm::mat4(1.0f);

        [[nodiscard]] AABB transform() const noexcept { return local.transform(transformMatrix); }

        bool operator==(const OBB&) const noexcept = default;
    };

    struct Plane {
        glm::vec3 normal;
        float d;
    };

    [[nodiscard]] static constexpr bool within(const float value, const float minBound, const float maxBound) noexcept {
        return value >= minBound && value <= maxBound;
    }
};
