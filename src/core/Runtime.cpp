#include "Runtime.h"

#include "engine/Engine.h"

#include "core/resources/AssetManager.h"
#include "entities/objects/ObjectManager.h"

#include "graphics/vulkan/VulkanRenderer.h"

#include <chrono>
#include <thread>

Runtime::Runtime(std::atomic<bool>& runningFlag)
    : _running(runningFlag),
      _window(1280, 720, "Noble Engine"),
      _renderer(Engine::MAX_FRAMES_IN_FLIGHT)
{

}

bool Runtime::init(std::string& errorMessage) {
    _window.show();

    _inputManager.init(_window.handle());

    _windowContext = WindowContext{&_window, &_inputManager};
    _window.setContext(&_windowContext);

    _objectManager.addObject("lucy.obj", {-1.0f, 1.0f, 1.47f}, {0.0f, 0.0f, -30.0f}, glm::vec3{0.0025f});
    _objectManager.addObject("stanford_dragon.obj", {3.0f, 0.7f, 0.6f}, {0.0f, 180.0f, 60.0f}, glm::vec3{0.6f});
    _objectManager.addObject("stanford_bunny.obj", {-3.0f, 1.0f, -0.25f}, {90.0f, 90.0f, 0.0f}, glm::vec3{7.0f});
    _objectManager.addObject("happy.obj", {-4.5f, -0.4f, -0.36f}, {90.0f, 120.0f, 0.0f}, glm::vec3{7.0f});
    _objectManager.addObject("sponza_old.gltf", {0.0f, 0.0f, 0.0f}, {90.0f, 0.0f, 0.0f}, glm::vec3{1.0f});

    _objectManager.createObjects();

    if (!_renderer.init(_window, _assetManager, _objectManager, errorMessage)) {
        return false;
    }

    _camera.setController(std::make_unique<CameraController>(_window.handle(), _inputManager, _camera));

    return true;
}

void Runtime::shutdown() {
    _running.store(false, std::memory_order_relaxed);

    _renderer.shutdown();
}

void Runtime::run() {
    // Engine thread (draws, inputs, updates)
    _engineThread = std::thread(&Runtime::engineLoop, this);

    // Window events polling in the main thread
    while (_running && !_window.shouldClose()) {
        _window.pollEvents();
    }

    _running.store(false, std::memory_order_relaxed);

    if (_engineThread.joinable()) {
        _engineThread.join();
    }
}

void Runtime::engineLoop() {
    using highResolutionClock = std::chrono::high_resolution_clock;

    auto previousTime  = highResolutionClock::now();
    auto lastFpsUpdate = previousTime;

    int frameCount = 0;
    int framerate  = 0;

    while (_running) {
        auto         currentTime = highResolutionClock::now();
        const double deltaTime   = std::chrono::duration<double>(currentTime - previousTime).count();
        previousTime = currentTime;

        _inputManager.update();

        _camera.update(deltaTime);
        _renderer.drawFrame(_camera);

        ++frameCount;

        const double timeSinceLastUpdate = std::chrono::duration<double>(currentTime - lastFpsUpdate).count();
        if (timeSinceLastUpdate >= 1) {
            framerate     = static_cast<int>(frameCount / timeSinceLastUpdate);
            frameCount    = 0;
            lastFpsUpdate = currentTime;

            _window.setTitle(
                "Noble Engine | " + std::to_string(framerate) + " FPS" + " | " +
                std::to_string(_renderer.primitiveCount) + " Triangles"
            );
        }

        std::this_thread::yield();
    }
}
