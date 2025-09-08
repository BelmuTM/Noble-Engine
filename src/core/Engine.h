#pragma once
#ifndef BAZARENGINE_ENGINE_H
#define BAZARENGINE_ENGINE_H

// Behavior macros
#define LOG_FILE_WRITE

#include <ctime>
#include <string>

namespace Engine {
    void localtime(std::tm& tm, const std::time_t* time);

    [[noreturn]] void fatalExit(const std::string& message);
}

#endif //BAZARENGINE_ENGINE_H