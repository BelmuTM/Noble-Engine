#pragma once
#ifndef NOBLEENGINE_CAMERA_H
#define NOBLEENGINE_CAMERA_H

#include "CameraController.h"

#include <algorithm>
#include <memory>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera()  = default;
    ~Camera() = default;

    Camera(const Camera&)            = delete;
    Camera& operator=(const Camera&) = delete;

    Camera(Camera&&)            = delete;
    Camera& operator=(Camera&&) = delete;

    void setController(std::unique_ptr<CameraController> controller) {
        _controller = std::move(controller);
    }

    [[nodiscard]] float getSpeed() const { return _speed; }
    [[nodiscard]] double getPitch() const { return _pitch; }
    [[nodiscard]] double getYaw() const { return _yaw; }
    [[nodiscard]] float getRotationSensitivity() const { return _rotationSensitivity; }
    [[nodiscard]] glm::vec3 getVelocity() const { return _velocity; }

    void setSpeed(const float speed) { _speed = speed; }
    void setYaw(const double yaw) { _yaw = yaw; }
    void setPitch(const double pitch) { _pitch = pitch; }
    void setVelocity(const glm::vec3& velocity) { _velocity = velocity; }

    void addFOV(const float delta) {
        _fov = std::clamp(_fov + delta, MIN_FOV, MAX_FOV);
    }

    void addSpeed(const float delta) { _speed += delta; }
    void addYaw(const float delta) { _yaw += delta; }
    void addPitch(const float delta) {
        _pitch = std::clamp(_pitch + delta, -89.999, 89.999);
    }
    void addVelocity(const glm::vec3& delta) { _velocity += delta; }

    void move(const glm::vec3& delta) { _position += delta; }

    [[nodiscard]] glm::mat4 getViewMatrix() const { return _viewMatrix; }

    [[nodiscard]] glm::mat4 getProjectionMatrix(const float aspectRatio) const {
        return glm::perspective(glm::radians(_fov), aspectRatio, _nearPlane, _farPlane);
    }

    void updateRotation();

    void update(double deltaTime);

private:
    std::unique_ptr<CameraController> _controller;

    float _fov = 70.0f;
    static constexpr float MIN_FOV = 30.0f;
    static constexpr float MAX_FOV = 120.0f;

    float _speed = 5.0f;

    float _nearPlane = 0.1f;
    float _farPlane  = 100.0f;

    glm::vec3 _velocity = {0.0f, 0.0f, 0.0f};

    glm::vec3 _position = {3.0f, 3.0f, 2.0f};
    glm::vec3 _target   = {0.0f, 0.0f, 0.0f};
    glm::vec3 _up       = {0.0f, 0.0f, 1.0f};

    glm::vec3 _forward = glm::normalize(_target - _position);

    double _yaw   = glm::degrees(atan2(_forward.y, _forward.x));
    double _pitch = glm::degrees(asin(_forward.z));

    float _rotationSensitivity = 0.2f;

    glm::mat4 _viewMatrix = {1.0f};

    void updateViewMatrix();

    [[nodiscard]] glm::vec3 toWorldSpace(glm::vec3 vector) const;
};

#endif //NOBLEENGINE_CAMERA_H
