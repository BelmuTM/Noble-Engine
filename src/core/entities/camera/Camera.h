#pragma once

#include "CameraController.h"

#include "common/Math.h"

#include <algorithm>
#include <memory>

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

    void move(const glm::vec3& delta) noexcept {
        _position += delta;

        _viewMatrixDirty = true;
    }

    void updateRotation();

    void update(double deltaTime);

    void setController(std::unique_ptr<CameraController> controller) {
        _controller = std::move(controller);
    }

    [[nodiscard]] float getNearPlane() const noexcept { return _nearPlane; }
    [[nodiscard]] float getFarPlane() const noexcept { return _farPlane; }

    [[nodiscard]] double getPitch() const noexcept { return _pitch; }
    [[nodiscard]] double getYaw() const noexcept { return _yaw; }
    [[nodiscard]] float getRotationSensitivity() const noexcept { return _rotationSensitivity; }

    [[nodiscard]] float getSpeed() const noexcept { return _speed; }
    [[nodiscard]] glm::vec3 getVelocity() const noexcept { return _velocity; }
    [[nodiscard]] glm::vec3 getPosition() const noexcept { return _position; }

    [[nodiscard]] const glm::mat4& getViewMatrix() noexcept;
    [[nodiscard]] const glm::mat4& getViewInverseMatrix() noexcept;

    [[nodiscard]] const glm::mat4& getProjectionMatrix() noexcept;
    [[nodiscard]] const glm::mat4& getProjectionInverseMatrix() noexcept;

    void setAspectRatio(const float aspectRatio) noexcept {
        if (std::abs(_aspectRatio - aspectRatio) > Math::EPSILON) {
            _aspectRatio = aspectRatio;

            _projectionMatrixDirty = true;
        }
    }

    void setFOV(const float fov) noexcept {
        if (std::abs(_fov - fov) > Math::EPSILON) {
            _fov = std::clamp(fov, MIN_FOV, MAX_FOV);

            _projectionMatrixDirty = true;
        }
    }

    void setSpeed(const float speed) noexcept { _speed = speed; }
    void setYaw(const double yaw) noexcept { _yaw = yaw; }
    void setPitch(const double pitch) noexcept { _pitch = pitch; }
    void setVelocity(const glm::vec3& velocity) noexcept { _velocity = velocity; }

    void addFOV(const float delta) noexcept { setFOV(_fov + delta); }

    void addYaw(const float delta) noexcept { _yaw += delta; }
    void addPitch(const float delta) noexcept {
        _pitch = std::clamp(_pitch + delta, -89.999, 89.999);
    }

    void addSpeed(const float delta) noexcept { _speed += delta; }
    void addVelocity(const glm::vec3& delta) noexcept { _velocity += delta; }

private:
    [[nodiscard]] glm::vec3 toWorldSpace(glm::vec3 vector) const;

    std::unique_ptr<CameraController> _controller{};

    float _aspectRatio = 1.0f;

    float _fov = 70.0f;
    static constexpr float MIN_FOV = 30.0f;
    static constexpr float MAX_FOV = 140.0f;

    float _nearPlane = 0.01f;
    float _farPlane  = 100.0f;

    double _yaw   = glm::degrees(atan2(_forward.y, _forward.x));
    double _pitch = glm::degrees(asin(_forward.z));

    float _rotationSensitivity = 0.2f;

    float _speed = 5.0f;

    glm::vec3 _velocity = {0.0f, 0.0f, 0.0f};

    glm::vec3 _position = {3.0f, 0.0f, 3.0f};
    glm::vec3 _target   = {0.0f, 0.0f, 0.0f};
    glm::vec3 _up       = {0.0f, 0.0f, 1.0f};

    glm::vec3 _forward = glm::normalize(_target - _position);

    glm::mat4 _viewMatrix{1.0f};
    glm::mat4 _viewInverseMatrix{1.0f};
    bool _viewMatrixDirty  = true;
    bool _viewInverseValid = false;

    glm::mat4 _projectionMatrix{0.0f};
    glm::mat4 _projectionInverseMatrix{0.0f};
    bool _projectionMatrixDirty  = true;
    bool _projectionInverseValid = false;
};
