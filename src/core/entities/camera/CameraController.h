#pragma once
#ifndef NOBLEENGINE_CAMERACONTROLLER_H
#define NOBLEENGINE_CAMERACONTROLLER_H

#include "core/input/InputManager.h"

#include <unordered_map>

#include <glm/vec3.hpp>

// TO-DO: Write controller interface ICameraController for different possible camera types

class Camera;

class CameraController final : public IInputListener {
public:
    explicit CameraController(GLFWwindow* window, InputManager& inputManager, Camera& camera);

    void update(float deltaTime);

    void onKeyEvent(int key, int action) override;
    void onMouseClick(int button, int action) override;
    void onMouseMove(double x, double y) override;
    void onMouseScroll(double offsetX, double offsetY) override;

private:
    GLFWwindow* _window = nullptr;
    Camera&     _camera;

    std::unordered_map<InputAction, glm::vec3> _actionMovementMap{};

    bool _dragging = false;
};

#endif // NOBLEENGINE_CAMERACONTROLLER_H
