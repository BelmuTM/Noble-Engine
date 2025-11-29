#pragma once
#ifndef NOBLEENGINE_ENGINE_H
#define NOBLEENGINE_ENGINE_H

// Behavior macros
#define LOG_FILE_WRITE

#include <cstdint>
#include <ctime>
#include <string>

namespace Engine {
    inline static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    void localtime(std::tm& tm, const std::time_t* time);

    [[noreturn]] void fatalExit(const std::string& message);
}

#endif //NOBLEENGINE_ENGINE_H