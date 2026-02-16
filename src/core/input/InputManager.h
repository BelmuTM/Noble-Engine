#pragma once

#include <unordered_map>
#include <unordered_set>
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
    MoveDown,
    IncreaseSpeed
};

class InputManager {
public:
    InputManager()  = default;
    ~InputManager() = default;

    void init(GLFWwindow* window);

    void initKeyBindings();

    void update();

    void onKeyEvent(int key, int action);
    void onMouseClick(int button, int action);
    void onMouseMove(double x, double y);
    void onMouseScroll(double offsetX, double offsetY) const;

    void addListener(IInputListener* listener) { _listeners.push_back(listener); }

    void bindKey(const int key, const InputAction action) {
        _keyBindings[key] = action;
    }

    [[nodiscard]] InputAction getKeybindAction(const int key) const {
        return _keyBindings.contains(key) ? _keyBindings.at(key) : InputAction::None;
    }

    [[nodiscard]] std::unordered_set<InputAction> getPressedActions() const noexcept { return _pressedActions; }

    [[nodiscard]] bool isKeyPressed(const int key) const {
        return _currentKeys.contains(key) && _currentKeys.at(key);
    }

    [[nodiscard]] bool isActionPressed(const InputAction action) const {
        return _pressedActions.contains(action);
    }

    [[nodiscard]] bool isMouseLeftButtonPressed() const noexcept { return _mouseLeftButtonPressed; }
    [[nodiscard]] bool isMouseRightButtonPressed() const noexcept { return _mouseRightButtonPressed; }

    [[nodiscard]] glm::vec2 getMousePosition() const noexcept { return _mousePosition; }
    [[nodiscard]] glm::vec2 getLastMousePosition() const noexcept { return _lastMousePosition; }
    [[nodiscard]] glm::vec2 getMouseDelta() const noexcept { return _mouseDelta; }

private:
    std::vector<IInputListener*> _listeners{};

    std::unordered_map<int, InputAction> _keyBindings{};
    std::unordered_map<int, bool>        _currentKeys{};
    std::unordered_set<InputAction>      _pressedActions{};

    glm::vec2 _mousePosition{0.0f};
    glm::vec2 _lastMousePosition{0.0f};
    glm::vec2 _mouseDelta{0.0f};

    bool _mouseLeftButtonPressed  = false;
    bool _mouseRightButtonPressed = false;
};

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
