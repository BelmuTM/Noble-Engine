#include "Engine.h"
#include "debug/Logger.h"
#include "platform/Platform.h"

#include <chrono>
#include <iostream>

#include "graphics/vulkan/VulkanContext.h"

int main() {
    Logger::Manager loggerManager;

    if (!Platform::init()) {
        Engine::fatalExit(ERROR_MESSAGE(Platform::init));
    }

    const Platform::Window window(800, 600, "Noble Engine");
    window.show();

    using highResolutionClock = std::chrono::high_resolution_clock;

    VulkanContext vulkanContext;
    vulkanContext.init(window);

    auto previousTime  = highResolutionClock::now();
    auto lastFpsUpdate = previousTime;

    int frameCount = 0;
    int framerate  = 0;

    while (Platform::pollEvents()) {
        auto currentTime = highResolutionClock::now();

        frameCount++;

        const auto timeSinceLastUpdate =
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
