#include "CameraController.h"

#include "Camera.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

CameraController::CameraController(InputManager& inputManager, Camera& camera)
    : IInputListener(inputManager), _camera(camera) {

    _actionMap[InputAction::MoveForward]  = glm::vec3{ 1.0f, 0.0f, 0.0f};
    _actionMap[InputAction::MoveBackward] = glm::vec3{-1.0f, 0.0f, 0.0f};
    _actionMap[InputAction::MoveLeft]     = glm::vec3{ 0.0f,-1.0f, 0.0f};
    _actionMap[InputAction::MoveRight]    = glm::vec3{ 0.0f, 1.0f, 0.0f};
    _actionMap[InputAction::MoveUp]       = glm::vec3{ 0.0f, 0.0f, 1.0f};
    _actionMap[InputAction::MoveDown]     = glm::vec3{ 0.0f, 0.0f,-1.0f};
}

void CameraController::update() {
    glm::vec3 velocity{0.0f};

    for (auto action : _pressedActions) {
        if (_actionMap.contains(action))
            velocity += _actionMap[action];
    }

    if (glm::length(velocity) > 0.0f)
        velocity = glm::normalize(velocity);

    _camera.setVelocity(velocity);
}

void CameraController::onKeyEvent(const int key, const int action) {
    const InputAction act = _inputManager.getKeybind(key);
    if (act == InputAction::None) return;

    if (action == GLFW_PRESS)
        _pressedActions.insert(act);
    else if (action == GLFW_RELEASE)
        _pressedActions.erase(act);
}

void CameraController::onMouseMove(double x, double y) {
}
