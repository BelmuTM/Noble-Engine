#pragma once

namespace HashUtils {
    template<typename T>
    void combine(std::size_t& seed, const T& value) noexcept {
        static constexpr std::size_t goldenRatio = 0x9E3779B9;
        seed ^= std::hash<T>{}(value) + goldenRatio + (seed << 6) + (seed >> 2);
    }

    inline void combine(std::size_t& seed, const glm::vec2& v) noexcept {
        combine(seed, v.x);
        combine(seed, v.y);
    }

    inline void combine(std::size_t& seed, const glm::vec3& v) noexcept {
        combine(seed, v.x);
        combine(seed, v.y);
        combine(seed, v.z);
    }

    inline void combine(std::size_t& seed, const glm::vec4& v) noexcept {
        combine(seed, v.x);
        combine(seed, v.y);
        combine(seed, v.z);
        combine(seed, v.w);
    }
}
