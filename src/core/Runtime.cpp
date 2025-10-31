#include "Engine.h"
#include "Platform.h"
#include "WindowContext.h"
#include "debug/Logger.h"

#include "graphics/vulkan/core/VulkanRenderer.h"

#include <atomic>
#include <chrono>
#include <thread>

int main() {
    Logger::Manager loggerManager;

    if (!Platform::init()) {
        Engine::fatalExit("Failed to init platform");
    }

    Platform::Window window(800, 600, "Noble Engine");
    window.show();

    InputManager inputManager;
    inputManager.init(window.handle());

    WindowContext ctx{&window, &inputManager};
    glfwSetWindowUserPointer(window.handle(), &ctx);

    VulkanRenderer renderer;
    if (!renderer.init(window)) {
        Engine::fatalExit("Failed to init Vulkan renderer");
    }

    Camera camera;
    camera.setController(std::make_unique<CameraController>(inputManager, camera));

    std::atomic running(true);

    std::thread engineThread([&] {
        using highResolutionClock = std::chrono::high_resolution_clock;

        auto previousTime  = highResolutionClock::now();
        auto lastFpsUpdate = previousTime;

        int frameCount = 0;
        int framerate  = 0;

        while (running) {
            auto         currentTime = highResolutionClock::now();
            const double deltaTime   = std::chrono::duration<double>(currentTime - previousTime).count();
            previousTime = currentTime;

            inputManager.update();

            camera.update(deltaTime);
            renderer.drawFrame(camera);

            ++frameCount;

            const double timeSinceLastUpdate = std::chrono::duration<double>(currentTime - lastFpsUpdate).count();
            if (timeSinceLastUpdate >= 1) {
                framerate     = static_cast<int>(frameCount / timeSinceLastUpdate);
                frameCount    = 0;
                lastFpsUpdate = currentTime;

                window.setTitle("Noble Engine | " + std::to_string(framerate) + " fps");
            }

            std::this_thread::yield();
        }
    });

    window.pollEvents();

    running.store(false);
    engineThread.join();

    Platform::shutdown();

    return 0;
}
