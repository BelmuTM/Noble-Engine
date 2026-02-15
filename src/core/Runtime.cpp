#include "Runtime.h"

#include "engine/Engine.h"

#include "graphics/vulkan/VulkanRenderer.h"

#include <chrono>
#include <thread>

Runtime::Runtime(const Scene& scene, std::atomic<bool>& runningFlag)
    : _running(runningFlag),
      _window(1280, 720, "Noble Engine"),
      _renderer(Engine::MAX_FRAMES_IN_FLIGHT)
{
    _objectManager.addScene(scene);
}

bool Runtime::init(std::string& errorMessage) {
    _window.show();

    _inputManager.init(_window.handle());

    _windowContext = WindowContext{&_window, &_inputManager};
    _window.setContext(&_windowContext);

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
