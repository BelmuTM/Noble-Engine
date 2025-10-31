#pragma once
#ifndef NOBLEENGINE_CAMERACONTROLLER_H
#define NOBLEENGINE_CAMERACONTROLLER_H

#include "core/InputManager.h"

#include <unordered_map>
#include <unordered_set>

#include <glm/vec3.hpp>

// TO-DO: Write controller interface ICameraController for different possible camera types

class Camera;

class CameraController final : public IInputListener {
public:
    explicit CameraController(InputManager& inputManager, Camera& camera);

    void update();

    void onKeyEvent(int key, int action) override;
    void onMouseMove(double x, double y) override;

private:
    Camera& _camera;

    std::unordered_set<InputAction>            _pressedActions;
    std::unordered_map<InputAction, glm::vec3> _actionMap;
};

#endif // NOBLEENGINE_CAMERACONTROLLER_H
