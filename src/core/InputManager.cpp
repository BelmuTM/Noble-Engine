#include "InputManager.h"

#include "WindowContext.h"

void InputManager::init(GLFWwindow* window) {
    glfwSetKeyCallback(window, [](GLFWwindow* _window, const int _key, int, const int _action, int) {
        auto* self = static_cast<WindowContext*>(glfwGetWindowUserPointer(_window));
        if (self) self->inputManager->onKeyEvent(_key, _action);
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow* _window, const double _xpos, const double _ypos) {
        auto* self = static_cast<WindowContext*>(glfwGetWindowUserPointer(_window));
        if (self) self->inputManager->onMouseMove(_xpos, _ypos);
    });

    initKeyBindings();
}

void InputManager::initKeyBindings() {
    bindKey(GLFW_KEY_W, InputAction::MoveForward);
    bindKey(GLFW_KEY_A, InputAction::MoveLeft);
    bindKey(GLFW_KEY_S, InputAction::MoveBackward);
    bindKey(GLFW_KEY_D, InputAction::MoveRight);
    bindKey(GLFW_KEY_SPACE, InputAction::MoveUp);
    bindKey(GLFW_KEY_LEFT_CONTROL, InputAction::MoveDown);
}

void InputManager::onKeyEvent(const int key, const int action) {
    if (action == GLFW_PRESS)
        _currentKeys[key] = true;
    else if (action == GLFW_RELEASE)
        _currentKeys[key] = false;

    for (auto* listener : _listeners) {
        listener->onKeyEvent(key, action);
    }
}

void InputManager::onMouseMove(const double x, const double y) {
    _mousePosition = {x, y};

    for (auto* listener : _listeners) {
        listener->onMouseMove(x, y);
    }
}

bool InputManager::isKeyPressed(const int key) const {
    return _currentKeys.contains(key) && _currentKeys.at(key);
}

void InputManager::update() {
    _mouseDelta        = _mousePosition - _lastMousePosition;
    _lastMousePosition = _mousePosition;
}
