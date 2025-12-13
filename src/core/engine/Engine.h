#pragma once
#ifndef NOBLEENGINE_ENGINE_H
#define NOBLEENGINE_ENGINE_H

#include <string>

namespace Engine {
    inline static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    [[noreturn]] void fatalExit(const std::string& message);
}

#endif //NOBLEENGINE_ENGINE_H