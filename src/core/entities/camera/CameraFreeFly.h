#pragma once

#include "ICameraBehavior.h"

#include "core/input/InputManager.h"

#include <unordered_map>

#include <glm/vec3.hpp>

class CameraFreeFly final : public ICameraBehavior, public IInputListener {
public:
    explicit CameraFreeFly(Camera& camera, InputManager& inputManager, GLFWwindow* window);

    void update(float deltaTime) override;

    void onKeyEvent(int key, int action) override;
    void onMouseClick(int button, int action) override;
    void onMouseMove(double x, double y) override;
    void onMouseScroll(double offsetX, double offsetY) override;

private:
    GLFWwindow* _window = nullptr;

    std::unordered_map<InputAction, glm::vec3> _actionMovementMap{};

    bool _dragging = false;
};
