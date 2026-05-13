#include "Runtime.h"

#include "engine/Engine.h"

#include "graphics/vulkan/VulkanRenderer.h"

#include <chrono>
#include <thread>

#include "entities/camera/CameraFreeFly.h"

Runtime::Runtime(const Scene& scene, std::atomic<bool>& runningFlag)
    : _running(runningFlag),
      _window(1280, 720, "Noble Engine"),
      _renderer(Engine::MAX_FRAMES_IN_FLIGHT)
{
    _objectManager.addScene(scene);
}

Expected<void> Runtime::init() {
    _window.show();

    _inputManager.init(_window.handle());

    // Keybinds
    KeyMap keyMap{};
    keyMap.bind(GLFW_KEY_W, InputAction::MoveForward);
    keyMap.bind(GLFW_KEY_A, InputAction::MoveLeft);
    keyMap.bind(GLFW_KEY_S, InputAction::MoveBackward);
    keyMap.bind(GLFW_KEY_D, InputAction::MoveRight);
    keyMap.bind(GLFW_KEY_SPACE, InputAction::MoveUp);
    keyMap.bind(GLFW_KEY_LEFT_CONTROL, InputAction::MoveDown);
    keyMap.bind(GLFW_KEY_LEFT_SHIFT, InputAction::IncreaseSpeed);
    keyMap.bind(GLFW_KEY_TAB, InputAction::ToggleDebugView);

    _inputManager.setKeyMap(keyMap);

    _windowContext = WindowContext{&_window, &_inputManager};
    _window.setContext(&_windowContext);

    _objectManager.createObjects();

    TRY(_renderer.init(_window, _assetManager, _objectManager));

    _cameraBehavior = std::make_unique<CameraFreeFly>(_camera, _inputManager, _window.handle());

    return {};
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

        if (_inputManager.isPressed(InputAction::ToggleDebugView)) {
            _debugState.incrementMode();

            Logger::debug("Set debug mode to " + std::to_string(_debugState.debugMode));
        }

        _inputManager.update();

        int windowWidth, windowHeight;
        _window.getFramebufferSize(windowWidth, windowHeight);

        _cameraBehavior->update(deltaTime);
        _camera.setAspectRatio(static_cast<float>(windowWidth) / static_cast<float>(windowHeight));
        _uniforms.update(_camera, windowWidth, windowHeight, _debugState);

        auto frameDraw = _renderer.drawFrame(_uniforms);
        if (frameDraw.failed()) Logger::error(frameDraw.failure());

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
