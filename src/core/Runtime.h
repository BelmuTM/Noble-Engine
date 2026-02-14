#pragma once
#ifndef NOBLEENGINE_RUNTIME_H
#define NOBLEENGINE_RUNTIME_H

#include "platform/Window.h"
#include "platform/WindowContext.h"

#include "graphics/vulkan/VulkanRenderer.h"

#include <atomic>
#include <string>

class Runtime {
public:
    explicit Runtime(std::atomic<bool>& runningFlag);

    ~Runtime() = default;

    [[nodiscard]] bool init(std::string& errorMessage);

    void shutdown();

    void run();

private:
    void engineLoop();

    std::atomic<bool>& _running;

    Window        _window;
    InputManager  _inputManager{};
    WindowContext _windowContext{};

    Camera         _camera{};
    VulkanRenderer _renderer{};

    AssetManager  _assetManager{};
    ObjectManager _objectManager{_assetManager};

    std::thread _engineThread;
};

#endif // NOBLEENGINE_RUNTIME_H
