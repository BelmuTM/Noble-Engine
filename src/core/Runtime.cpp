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

    Object dragon;
    dragon.create("stanford_dragon.obj", "viking_room.png", {0.0f, 0.0f, 0.0f}, {0.0f, 180.0f, 0.0f}, {1.0f, 1.0f, 1.0f});

    Object dragon2;
    dragon2.create("stanford_dragon.obj", "viking_room.png", {2.0f, 2.0f, -0.4f}, {0.0f, 180.0f, 40.0f}, {0.6f, 0.6f, 0.6f});

    Object dragon3;
    dragon3.create("stanford_dragon.obj", "viking_room.png", {2.0f, -2.0f, -0.6f}, {0.0f, 180.0f, -25.0f}, {0.4f, 0.4f, 0.4f});

    std::vector objects{dragon, dragon2, dragon3};

    VulkanRenderer renderer;
    if (!renderer.init(window, objects)) {
        Engine::fatalExit("Failed to init Vulkan renderer");
    }

    Camera camera;
    camera.setController(std::make_unique<CameraController>(window.handle(), inputManager, camera));

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
