#pragma once

#include "engine/Engine.h"

#include "platform/Window.h"
#include "platform/WindowContext.h"

#include "input/InputManager.h"

#include "entities/camera/Camera.h"
#include "entities/camera/ICameraBehavior.h"

#include "core/resources/AssetManager.h"
#include "entities/objects/ObjectManager.h"

#include "graphics/vulkan/VulkanRenderer.h"

#include <atomic>

class Runtime {
public:
    explicit Runtime(const Scene& scene, std::atomic<bool>& runningFlag);

    ~Runtime() = default;

    [[nodiscard]] Expected<void> init();

    void shutdown();

    void run();

private:
    void renderLoop();

    std::atomic<bool>& _running;

    DebugState _debugState{};

    Window        _window;
    InputManager  _inputManager{};
    WindowContext _windowContext{};

    Camera                           _camera{};
    std::unique_ptr<ICameraBehavior> _cameraBehavior{};

    struct FramePacket {
        FrameUniforms uniforms;
    };

    std::array<FramePacket, Engine::MAX_FRAMES_IN_FLIGHT> _framePackets{};

    std::atomic<std::uint32_t> _producedFrame{0};
    std::atomic<std::uint32_t> _consumedFrame{0};

    std::atomic<std::uint32_t> _framerate;

    VulkanRenderer _renderer{};

    AssetManager  _assetManager{};
    ObjectManager _objectManager{_assetManager};

    std::thread _renderThread;
};
