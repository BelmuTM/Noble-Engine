#include "CameraController.h"

#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

static constexpr float speedMultiplier = 3.0f;
static constexpr float cameraSmoothing = 6.0f;

CameraController::CameraController(GLFWwindow* window, InputManager& inputManager, Camera& camera)
    : IInputListener(inputManager), _window(window), _camera(camera)
{
    _actionMovementMap[InputAction::MoveForward]  = glm::vec3{ 1.0f, 0.0f, 0.0f};
    _actionMovementMap[InputAction::MoveBackward] = glm::vec3{-1.0f, 0.0f, 0.0f};
    _actionMovementMap[InputAction::MoveLeft]     = glm::vec3{ 0.0f,-1.0f, 0.0f};
    _actionMovementMap[InputAction::MoveRight]    = glm::vec3{ 0.0f, 1.0f, 0.0f};
    _actionMovementMap[InputAction::MoveUp]       = glm::vec3{ 0.0f, 0.0f, 1.0f};
    _actionMovementMap[InputAction::MoveDown]     = glm::vec3{ 0.0f, 0.0f,-1.0f};
}

void CameraController::update(const float deltaTime) {
    glm::vec3 velocity{0.0f};

    // Movement
    for (auto action : _inputManager.getPressedActions()) {
        if (_actionMovementMap.contains(action))
            velocity += _actionMovementMap[action];
    }

    if (glm::length(velocity) > 0.0f) {
        velocity = glm::normalize(velocity);
    }

    float speed = _camera.getSpeed();
    if (_inputManager.isActionPressed(InputAction::IncreaseSpeed)) {
        speed *= speedMultiplier;
    }
    velocity *= speed;

    const glm::vec3 currentVelocity  = _camera.getVelocity();
    const glm::vec3 smoothedVelocity = glm::mix(currentVelocity, velocity, 1.0f - expf(-cameraSmoothing * deltaTime));

    _camera.setVelocity(smoothedVelocity);

    // Rotation
    if (_dragging) {
        const float sensitivity = _camera.getRotationSensitivity();

        const glm::vec2 delta = _inputManager.getMouseDelta();

        _camera.addYaw  (-delta.x * sensitivity);
        _camera.addPitch(-delta.y * sensitivity);
        _camera.updateRotation();
    }
}

void CameraController::onKeyEvent(const int key, const int action) {
}

void CameraController::onMouseClick(int button, int action) {
    _dragging = _inputManager.isMouseRightButtonPressed();

    const int cursorMode = _dragging ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;

    glfwSetInputMode(_window, GLFW_CURSOR, cursorMode);
}

void CameraController::onMouseMove(double x, double y) {
}

void CameraController::onMouseScroll(const double, const double offsetY) {
    _camera.addFOV(static_cast<float>(offsetY));
}
