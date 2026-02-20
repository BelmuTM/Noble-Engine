#pragma once

#include "KeyMap.h"

#include <unordered_set>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

class IInputListener;

class InputManager {
public:
    InputManager()  = default;
    ~InputManager() = default;

    void init(GLFWwindow* window) const;

    void update();

    void onKeyEvent(int key, int action);
    void onMouseClick(int button, int action);
    void onMouseMove(double x, double y);
    void onMouseScroll(double offsetX, double offsetY);

    void addListener(IInputListener* listener) { _listeners.push_back(listener); }

    [[nodiscard]] const KeyMap& getKeyMap() const noexcept { return _keyMap; }
    void setKeyMap(const KeyMap& keyMap) noexcept { _keyMap = keyMap; }

    [[nodiscard]] std::unordered_set<InputAction> getHeldInputs() const noexcept { return _heldInputs; }
    [[nodiscard]] std::unordered_set<InputAction> getPressedInputs() const noexcept { return _pressedInputs; }
    [[nodiscard]] std::unordered_set<InputAction> getReleasedInputs() const noexcept { return _releasedInputs; }

    [[nodiscard]] bool isHeld(const InputAction action) const { return _heldInputs.contains(action); }
    [[nodiscard]] bool isPressed(const InputAction action) const { return _pressedInputs.contains(action); }
    [[nodiscard]] bool isReleased(const InputAction action) const { return _releasedInputs.contains(action); }

    [[nodiscard]] bool isMouseLeftButtonPressed() const noexcept { return _mouseLeftButtonPressed; }
    [[nodiscard]] bool isMouseRightButtonPressed() const noexcept { return _mouseRightButtonPressed; }

    [[nodiscard]] glm::vec2 getMousePosition() const noexcept { return _mousePosition; }
    [[nodiscard]] glm::vec2 getLastMousePosition() const noexcept { return _lastMousePosition; }
    [[nodiscard]] glm::vec2 getMouseDelta() const noexcept { return _mouseDelta; }
    [[nodiscard]] glm::vec2 getMouseScrollDelta() const noexcept { return _scrollDelta; }

private:
    KeyMap _keyMap{};

    std::vector<IInputListener*> _listeners{};

    std::unordered_set<InputAction> _heldInputs{};
    std::unordered_set<InputAction> _pressedInputs{};
    std::unordered_set<InputAction> _releasedInputs{};

    glm::vec2 _mousePosition{0.0f};
    glm::vec2 _lastMousePosition{0.0f};
    glm::vec2 _mouseDelta{0.0f};
    glm::vec2 _scrollDelta{0.0f};

    bool _mouseLeftButtonPressed  = false;
    bool _mouseRightButtonPressed = false;
};

// TO-DO: Queue listener events to support multithreading.
class IInputListener {
public:
    explicit IInputListener(InputManager& inputManager) : _inputManager(inputManager) {
        _inputManager.addListener(this);
    }

    virtual ~IInputListener() = default;

    virtual void onKeyEvent(int key, int action) = 0;
    virtual void onMouseClick(int button, int action) = 0;
    virtual void onMouseMove(double x, double y) = 0;
    virtual void onMouseScroll(double offsetX, double offsetY) = 0;

protected:
    InputManager& _inputManager;
};
