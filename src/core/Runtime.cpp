#include "Engine.h"
#include "debug/Logger.h"
#include "platform/Platform.h"

#include <chrono>
#include <iostream>

#include "graphics/vulkan/VulkanContext.h"

int main() {
    Logger::Manager loggerManager;

    if (!Platform::init()) {
        Engine::fatalExit("Failed to init platform");
    }

    const Platform::Window window(800, 600, "BazarEngine");
    window.show();

    VulkanContext vulkanContext;
    vulkanContext.init();

    using high_resolution_clock = std::chrono::high_resolution_clock;

    auto previousTime  = high_resolution_clock::now();
    auto lastFpsUpdate = previousTime;

    int frameCount = 0;
    int framerate  = 0;

    while (Platform::pollEvents()) {
        auto currentTime = high_resolution_clock::now();

        frameCount++;

        auto timeSinceLastUpdate =
            std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastFpsUpdate).count();
        if (timeSinceLastUpdate >= 1) {
            framerate     = frameCount;
            frameCount    = 0;
            lastFpsUpdate = currentTime;

            Logger::info(std::to_string(framerate) + " fps");
        }

        previousTime = currentTime;
    }

    Platform::shutdown();

    return 0;
}
