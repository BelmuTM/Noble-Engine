#pragma once
#ifndef NOBLEENGINE_ENGINE_H
#define NOBLEENGINE_ENGINE_H

// Behavior macros
#define LOG_FILE_WRITE

#include <cstdint>
#include <ctime>
#include <string>

#include <vulkan/vulkan_core.h>

constexpr uint32_t VULKAN_VERSION = VK_API_VERSION_1_4;

static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

namespace Engine {
    void localtime(std::tm& tm, const std::time_t* time);

    [[noreturn]] void fatalExit(const std::string& message);
}

#endif //NOBLEENGINE_ENGINE_H