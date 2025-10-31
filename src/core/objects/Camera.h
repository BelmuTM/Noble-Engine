#pragma once
#ifndef NOBLEENGINE_CAMERA_H
#define NOBLEENGINE_CAMERA_H

#include "CameraController.h"

#include <memory>
#include <glm/glm.hpp>

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

    void setFOV(const float fov) { _fov = fov; }

    [[nodiscard]] float getSpeed() const { return _speed; }
    [[nodiscard]] double getYaw() const { return _yaw; }
    [[nodiscard]] double getPitch() const { return _pitch; }
    [[nodiscard]] glm::vec3 getVelocity() const { return _velocity; }

    void setSpeed(const float speed) { _speed = speed; }
    void setPitch(const double pitch) { _pitch = pitch; }
    void setYaw(const double yaw) { _yaw = yaw; }
    void setVelocity(const glm::vec3& velocity) { _velocity = velocity; }

    void addSpeed(const float delta) { _speed += delta; }
    void addPitch(const float delta) { _pitch += delta; }
    void addYaw(const float delta) { _yaw += delta; }
    void addVelocity(const glm::vec3& delta) { _velocity += delta; }

    void move(const glm::vec3& delta) { _position += delta; }

    [[nodiscard]] glm::mat4 getViewMatrix() const { return _viewMatrix; }
    [[nodiscard]] glm::mat4 getProjectionMatrix(float aspectRatio) const;

    void update(double deltaTime);

private:
    std::unique_ptr<CameraController> _controller;

    float _fov = 90.0f;

    float _speed = 5.0f;

    float _nearPlane = 0.1f;
    float _farPlane  = 100.0f;

    double _pitch = glm::degrees(asin(_front.z));
    double _yaw   = glm::degrees(atan2(_front.y, _front.x));

    glm::vec3 _velocity = {0.0f, 0.0f, 0.0f};

    glm::vec3 _position = {3.0f, 3.0f, 2.0f};
    glm::vec3 _target   = {0.0f, 0.0f, 0.0f};
    glm::vec3 _up       = {0.0f, 0.0f, 1.0f};

    glm::vec3 _front = glm::normalize(_target - _position);

    glm::mat4 _viewMatrix = {1.0f};

    void updateViewMatrix();
};

#endif //NOBLEENGINE_CAMERA_H
