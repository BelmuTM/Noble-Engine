#pragma once

#include "platform/Window.h"
#include "platform/WindowContext.h"

#include "input/InputManager.h"

#include "entities/camera/Camera.h"
#include "entities/camera/ICameraBehavior.h"

#include "core/resources/AssetManager.h"
#include "entities/objects/ObjectManager.h"

#include "graphics/vulkan/VulkanRenderer.h"

#include <atomic>
#include <string>

class Runtime {
public:
    explicit Runtime(const Scene& scene, std::atomic<bool>& runningFlag);

    ~Runtime() = default;

    [[nodiscard]] bool init(std::string& errorMessage);

    void shutdown();

    void run();

private:
    void engineLoop();

    std::atomic<bool>& _running;

    DebugState _debugState{};

    Window        _window;
    InputManager  _inputManager{};
    WindowContext _windowContext{};

    Camera                           _camera{};
    std::unique_ptr<ICameraBehavior> _cameraBehavior{};

    FrameUniforms  _uniforms{};
    VulkanRenderer _renderer{};

    AssetManager  _assetManager{};
    ObjectManager _objectManager{_assetManager};

    std::thread _engineThread;
};
