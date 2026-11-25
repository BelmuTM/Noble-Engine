#pragma once

#include "core/render/DrawFlags.h"

enum class CullMode : std::uint8_t {
    None,
    Frustum
};

struct DrawListDescriptor {
    DrawFlags drawFlags{};

    CullMode cullMode = CullMode::None;

    bool operator==(const DrawListDescriptor&) const = default;
};

using DrawListId = std::uint32_t;
