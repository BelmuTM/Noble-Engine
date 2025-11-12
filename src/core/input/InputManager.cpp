#include "InputManager.h"

#include "core/WindowContext.h"

void keyCallback(GLFWwindow* _window, const int _key, const int _scancode, const int _action, const int _mods) {
    const auto* self = static_cast<WindowContext*>(glfwGetWindowUserPointer(_window));
    if (self) self->inputManager->onKeyEvent(_key, _action);
}

void mouseButtonCallback(GLFWwindow* _window, const int _button, const int _action, const int _mods) {
    const auto* self = static_cast<WindowContext*>(glfwGetWindowUserPointer(_window));
    if (self) self->inputManager->onMouseClick(_button, _action);
}

void cursorsPosCallback(GLFWwindow* _window, const double _xpos, const double _ypos) {
    const auto* self = static_cast<WindowContext*>(glfwGetWindowUserPointer(_window));
    if (self) self->inputManager->onMouseMove(_xpos, _ypos);
}

void scrollCallback(GLFWwindow* _window, const double _xoffset, const double _yoffset) {
    const auto* self = static_cast<WindowContext*>(glfwGetWindowUserPointer(_window));
    if (self) self->inputManager->onMouseScroll(_xoffset, _yoffset);
}

void InputManager::init(GLFWwindow* window) {
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorsPosCallback);
    glfwSetScrollCallback(window, scrollCallback);

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
    const InputAction act = getKeybindAction(key);

    if (action == GLFW_PRESS) {
        _currentKeys[key] = true;
        _pressedActions.insert(act);
    } else if (action == GLFW_RELEASE) {
        _currentKeys[key] = false;
        _pressedActions.erase(act);
    }

    for (auto* listener : _listeners) {
        listener->onKeyEvent(key, action);
    }
}

void InputManager::onMouseClick(const int button, const int action) {
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        _mouseLeftButtonPressed = action == GLFW_PRESS;
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
        _mouseRightButtonPressed = action == GLFW_PRESS;

    for (auto* listener : _listeners) {
        listener->onMouseClick(button, action);
    }
}

void InputManager::onMouseMove(const double x, const double y) {
    _mousePosition = {x, y};

    for (auto* listener : _listeners) {
        listener->onMouseMove(x, y);
    }
}

void InputManager::onMouseScroll(const double offsetX, const double offsetY) const {
    for (auto* listener : _listeners) {
        listener->onMouseScroll(offsetX, offsetY);
    }
}

bool InputManager::isKeyPressed(const int key) const {
    return _currentKeys.contains(key) && _currentKeys.at(key);
}

void InputManager::update() {
    _mouseDelta        = _mousePosition - _lastMousePosition;
    _lastMousePosition = _mousePosition;
}
