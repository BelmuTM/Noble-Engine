#include "Runtime.h"

#include "entities/camera/CameraFreeFly.h"

#include "multithreading/ThreadRegistry.h"

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

    _cameraBehavior = std::make_unique<CameraFreeFly>(_camera, _inputManager, _window.handle());

    TRY(_renderer.init(_window, _assetManager, _objectManager));

    return {};
}

void Runtime::shutdown() {
    _running.store(false, std::memory_order_relaxed);

    _renderer.shutdown();
}

void Runtime::run() {
    _renderThread = std::thread(&Runtime::renderLoop, this);

    using highResolutionClock = std::chrono::high_resolution_clock;

    auto previousTime = highResolutionClock::now();

    std::uint32_t frameIndex = 0;

    while (_running && !_window.shouldClose()) {
        _window.pollEvents();

        while (_producedFrame - _consumedFrame.load(std::memory_order_acquire) >= Engine::MAX_FRAMES_IN_FLIGHT) {
            std::this_thread::yield();
        }

        auto currentTime = highResolutionClock::now();

        const float deltaTime = std::chrono::duration<float>(currentTime - previousTime).count();

        previousTime = currentTime;

        if (_inputManager.isPressed(InputAction::ToggleDebugView)) {
            _debugState.incrementMode();

            Logger::debug("Set debug mode to " + std::to_string(_debugState.debugMode));
        }

        _inputManager.update();

        int windowWidth, windowHeight;
        _window.getFramebufferSize(windowWidth, windowHeight);

        _window.setTitle(
            "Noble Engine | " + std::to_string(_framerate.load(std::memory_order_relaxed)) + " FPS | " +
            std::to_string(_renderer.primitiveCount) + " Triangles"
        );

        _cameraBehavior->update(deltaTime);

        _camera.setAspectRatio(static_cast<float>(windowWidth) / static_cast<float>(windowHeight));

        auto& framePacket = _framePackets[frameIndex % Engine::MAX_FRAMES_IN_FLIGHT];

        framePacket.uniforms.update(
            _camera,
            windowWidth,
            windowHeight,
            _debugState
        );

        _producedFrame.fetch_add(1, std::memory_order_release);

        ++frameIndex;
    }

    _running.store(false, std::memory_order_relaxed);

    if (_renderThread.joinable()) {
        _renderThread.join();
    }
}

void Runtime::renderLoop() {
    ThreadScope renderScope("RenderThread");

    using highResolutionClock = std::chrono::high_resolution_clock;

    auto lastFpsUpdate = highResolutionClock::now();

    int frameCount = 0;
    uint32_t frameIndex = 0;

    while (_running) {
        // Wait for engine to produce a frame
        while (_consumedFrame >= _producedFrame.load(std::memory_order_acquire)) {
            if (!_running) return;
            std::this_thread::yield();
        }

        const auto& framePacket = _framePackets[frameIndex % Engine::MAX_FRAMES_IN_FLIGHT];

        auto frameDraw = _renderer.drawFrame(framePacket.uniforms);
        if (frameDraw.failed()) Logger::error(frameDraw.failure());

        _consumedFrame.fetch_add(1, std::memory_order_release);

        ++frameIndex;
        ++frameCount;

        const auto   currentTime         = highResolutionClock::now();
        const double timeSinceLastUpdate = std::chrono::duration<double>(currentTime - lastFpsUpdate).count();

        if (timeSinceLastUpdate >= 1) {
            _framerate.store(static_cast<int>(frameCount / timeSinceLastUpdate), std::memory_order_relaxed);

            frameCount    = 0;
            lastFpsUpdate = currentTime;
        }
    }
}
