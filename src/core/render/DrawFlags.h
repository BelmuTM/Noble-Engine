#pragma once

#include <cstdint>

class DrawFlags {
public:
    enum class Bit : std::uint32_t {
        Opaque = 1,
        Debug  = 1 << 1,
    };

    constexpr DrawFlags() = default;

    // Implicit on purpose
    constexpr DrawFlags(const Bit bit) noexcept : _bits(static_cast<std::uint32_t>(bit)) {}

    [[nodiscard]] friend constexpr DrawFlags operator|(const DrawFlags first, const DrawFlags second) noexcept {
        DrawFlags result;
        result._bits = first._bits | second._bits;
        return result;
    }

    [[nodiscard]] friend constexpr DrawFlags operator&(const DrawFlags first, const DrawFlags second) noexcept {
        DrawFlags result;
        result._bits = first._bits & second._bits;
        return result;
    }

    constexpr DrawFlags& operator|=(const DrawFlags other) noexcept {
        _bits |= other._bits;
        return *this;
    }

    [[nodiscard]] friend constexpr bool operator==(DrawFlags, DrawFlags) noexcept = default;

    [[nodiscard]] constexpr bool intersects(const DrawFlags other) const noexcept {
        return (_bits & other._bits) != 0;
    }

    [[nodiscard]] constexpr bool empty() const noexcept { return _bits == 0; }

private:
    std::uint32_t _bits = 0;
};
