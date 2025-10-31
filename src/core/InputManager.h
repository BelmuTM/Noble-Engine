#pragma once
#ifndef NOBLEENGINE_INPUTMANAGER_H
#define NOBLEENGINE_INPUTMANAGER_H

#include <unordered_map>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

class IInputListener;

enum class InputAction {
    None,
    MoveForward,
    MoveBackward,
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown
};

class InputManager {
public:
    InputManager()  = default;
    ~InputManager() = default;

    void init(GLFWwindow* window);

    void initKeyBindings();

    void bindKey(const int key, const InputAction action) { _keyBindings[key] = action; }

    [[nodiscard]] InputAction getKeybind(const int key) const {
        return _keyBindings.contains(key) ? _keyBindings.at(key) : InputAction::None;
    }

    void addListener(IInputListener* listener) { _listeners.push_back(listener); }

    void onKeyEvent(int key, int action);
    void onMouseMove(double x, double y);

    bool isKeyPressed(int key) const;

    void update();

    [[nodiscard]] glm::vec2 getMousePosition() const { return _mousePosition; }
    [[nodiscard]] glm::vec2 getMouseDelta() const { return _mouseDelta; }

private:
    std::vector<IInputListener*> _listeners;

    std::unordered_map<int, InputAction> _keyBindings{};
    std::unordered_map<int, bool>        _currentKeys{};

    glm::vec2 _mousePosition{0.0f};
    glm::vec2 _lastMousePosition{0.0f};
    glm::vec2 _mouseDelta{0.0f};
};

class IInputListener {
public:
    explicit IInputListener(InputManager& inputManager) : _inputManager(inputManager) {
        _inputManager.addListener(this);
    }
    virtual ~IInputListener() = default;

    virtual void onKeyEvent(int key, int action) = 0;
    virtual void onMouseMove(double x, double y) = 0;

protected:
    InputManager& _inputManager;
};

#endif // NOBLEENGINE_INPUTMANAGER_H
