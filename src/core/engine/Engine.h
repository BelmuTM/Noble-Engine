#pragma once

#include <string>

namespace Engine {
    inline static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    [[noreturn]] void fatalExit(const std::string& message);
}
