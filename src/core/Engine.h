#pragma once
#ifndef NOBLEENGINE_ENGINE_H
#define NOBLEENGINE_ENGINE_H

// Behavior macros
#define LOG_FILE_WRITE

#include <cstdint>
#include <ctime>
#include <string>

static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

namespace Engine {
    void localtime(std::tm& tm, const std::time_t* time);

    [[noreturn]] void fatalExit(const std::string& message);
}

#endif //NOBLEENGINE_ENGINE_H