#pragma once
#ifndef BAZARENGINE_ENGINE_H
#define BAZARENGINE_ENGINE_H

// Behavior macros
#define LOG_FILE_WRITE

#include <cstdint>
#include <ctime>
#include <string>

#include <vulkan/vulkan_core.h>

namespace Engine {
    constexpr uint32_t VULKAN_VERSION = VK_API_VERSION_1_4;

    void localtime(std::tm& tm, const std::time_t* time);

    [[noreturn]] void fatalExit(const std::string& message);
}

#endif //BAZARENGINE_ENGINE_H