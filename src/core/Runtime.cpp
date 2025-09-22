#include "Engine.h"
#include "debug/Logger.h"
#include "platform/Platform.h"

#include <atomic>
#include <chrono>
#include <thread>

#include "graphics/vulkan/core/VulkanRenderer.h"

int main() {
    Logger::Manager loggerManager;

    if (!Platform::init()) {
        Engine::fatalExit("Failed to init platform");
    }

    Platform::Window window(800, 600, "Noble Engine");
    window.show();

    VulkanRenderer renderer;
    if (!renderer.init(window)) {
        Engine::fatalExit("Failed to init Vulkan renderer");
    }

    std::atomic running(true);

    std::thread engineThread([&]() {
        using highResolutionClock = std::chrono::high_resolution_clock;

        auto previousTime  = highResolutionClock::now();
        auto lastFpsUpdate = previousTime;

        int frameCount = 0;
        int framerate  = 0;

        while (running) {
            auto currentTime = highResolutionClock::now();

            renderer.drawFrame();

            ++frameCount;

            const auto timeSinceLastUpdate =
                std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastFpsUpdate).count();
            if (timeSinceLastUpdate >= 1) {
                framerate     = frameCount;
                frameCount    = 0;
                lastFpsUpdate = currentTime;

                Logger::info(std::to_string(framerate) + " fps");
            }

            previousTime = currentTime;

            std::this_thread::yield();
        }
    });

    while (Platform::pollEvents()) {}

    running.store(false);
    engineThread.join();

    Platform::shutdown();

    return 0;
}
