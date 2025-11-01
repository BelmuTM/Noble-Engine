#include "CameraController.h"

#include "Camera.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

CameraController::CameraController(GLFWwindow* window, InputManager& inputManager, Camera& camera)
    : _window(window), IInputListener(inputManager), _camera(camera) {

    _actionMovementMap[InputAction::MoveForward]  = glm::vec3{ 1.0f, 0.0f, 0.0f};
    _actionMovementMap[InputAction::MoveBackward] = glm::vec3{-1.0f, 0.0f, 0.0f};
    _actionMovementMap[InputAction::MoveLeft]     = glm::vec3{ 0.0f,-1.0f, 0.0f};
    _actionMovementMap[InputAction::MoveRight]    = glm::vec3{ 0.0f, 1.0f, 0.0f};
    _actionMovementMap[InputAction::MoveUp]       = glm::vec3{ 0.0f, 0.0f, 1.0f};
    _actionMovementMap[InputAction::MoveDown]     = glm::vec3{ 0.0f, 0.0f,-1.0f};
}

void CameraController::update() {
    glm::vec3 velocity{0.0f};

    // Movement
    for (auto action : _inputManager.getPressedActions()) {
        if (_actionMovementMap.contains(action))
            velocity += _actionMovementMap[action];
    }

    if (glm::length(velocity) > 0.0f)
        velocity = glm::normalize(velocity);

    _camera.setVelocity(velocity);

    // Rotation
    if (_dragging) {
        const float sensitivity = _camera.getRotationSensitivity();

        const glm::vec2 delta = _inputManager.getMouseDelta();

        _camera.addYaw(-delta.x * sensitivity);
        _camera.addPitch(-delta.y * sensitivity);
        _camera.updateRotation();
    }
}

void CameraController::onKeyEvent(const int key, const int action) {
}

void CameraController::onMouseClick(int button, int action) {
    _dragging = _inputManager.isMouseRightButtonPressed();

    if (_dragging) {
        glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void CameraController::onMouseMove(double x, double y) {
}

void CameraController::onMouseScroll(const double, const double offsetY) {
    _camera.addFOV(static_cast<float>(offsetY));
}
