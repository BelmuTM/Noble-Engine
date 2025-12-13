#include "engine/Engine.h"
#include "debug/Logger.h"
#include "platform/Platform.h"
#include "platform/Window.h"
#include "platform/WindowContext.h"

#include "resources/images/ImageManager.h"
#include "resources/models/ModelManager.h"
#include "entities/objects/ObjectManager.h"

#include "graphics/vulkan/VulkanRenderer.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <thread>

std::atomic running(true);

void signalHandler(int signal) {
    running = false;
}

void setupSignalHandlers() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
}

#if defined(_WIN32) || defined(_WIN64)

BOOL WINAPI ConsoleHandler(const DWORD ctrlType) {
    switch (ctrlType) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            running = false;
            return TRUE;
        default:
            return FALSE;
    }
}

void setupConsoleHandler() {
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        Logger::error("Failed to set console control handler");
    }
}

#endif

int main() {
    setupSignalHandlers();

#if defined(_WIN32) || defined(_WIN64)
    setupConsoleHandler();
#endif

    std::string errorMessage;
    Logger::Manager loggerManager;

    if (!Platform::init(errorMessage)) {
        Engine::fatalExit(errorMessage);
    }

    Window window(1280, 720, "Noble Engine");
    window.show();

    InputManager inputManager{};
    inputManager.init(window.handle());

    WindowContext ctx{&window, &inputManager};
    glfwSetWindowUserPointer(window.handle(), &ctx);

    ModelManager modelManager{};
    ImageManager imageManager{};
    ObjectManager objectManager{&modelManager, &imageManager};

    objectManager.addObject("lucy.obj", {-1.0f, 1.0f, 1.47f}, {0.0f, 0.0f, -30.0f}, glm::vec3{0.0025f});
    objectManager.addObject("stanford_dragon.obj", {3.0f, 0.7f, 0.6f}, {0.0f, 180.0f, 60.0f}, glm::vec3{0.6f});
    objectManager.addObject("stanford_bunny.obj", {-3.0f, 1.0f, -0.25f}, {90.0f, 90.0f, 0.0f}, glm::vec3{7.0f});
    objectManager.addObject("happy.obj", {-4.5f, -0.4f, -0.36f}, {90.0f, 120.0f, 0.0f}, glm::vec3{7.0f});
    objectManager.addObject("sponza.gltf", {0.0f, 0.0f, 0.0f}, {90.0f, 0.0f, 0.0f}, glm::vec3{1.0f});

    objectManager.createObjects();

    VulkanRenderer renderer(Engine::MAX_FRAMES_IN_FLIGHT);
    if (!renderer.init(window, objectManager, errorMessage)) {
        Engine::fatalExit(errorMessage);
    }

    Camera camera{};
    camera.setController(std::make_unique<CameraController>(window.handle(), inputManager, camera));

    // Main engine thread (draw, updates)
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

                window.setTitle(
                    "Noble Engine | " + std::to_string(framerate) + " FPS" + " | " +
                    std::to_string(renderer.primitiveCount) + " Triangles"
                );
            }

            std::this_thread::yield();
        }

        window.close(); // running == false
    });

    window.pollEvents();

    running.store(false);
    engineThread.join();

    renderer.shutdown();

    Platform::shutdown();

    return 0;
}
