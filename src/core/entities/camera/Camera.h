#pragma once

#include "common/Math.h"

#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera {
public:
    Camera();
    ~Camera() = default;

    Camera(const Camera&)            = delete;
    Camera& operator=(const Camera&) = delete;

    Camera(Camera&&)            = delete;
    Camera& operator=(Camera&&) = delete;

    void move(const glm::vec3& delta) noexcept {
        _position += delta;

        _viewMatrixDirty = true;
    }

    void update(float deltaTime);

    void updateRotationBasis();

    [[nodiscard]] glm::vec3 toWorldSpace(glm::vec3 vector) const;

    // Getters

    [[nodiscard]] const glm::vec3& getPosition() const noexcept { return _position; }

    [[nodiscard]] const glm::quat& getOrientation() const noexcept { return _orientation; }

    [[nodiscard]] const glm::vec3& getForward() const noexcept { return _forward; }
    [[nodiscard]] const glm::vec3& getRight() const noexcept { return _right; }
    [[nodiscard]] const glm::vec3& getUp() const noexcept { return _up; }

    [[nodiscard]] const glm::vec3& getVelocity() const noexcept { return _velocity; }

    [[nodiscard]] float getSpeed() const noexcept { return _speed; }

    [[nodiscard]] float getRotationSensitivity() const noexcept { return _rotationSensitivity; }

    [[nodiscard]] float getNearPlane() const noexcept { return _nearPlane; }
    [[nodiscard]] float getFarPlane() const noexcept { return _farPlane; }

    [[nodiscard]] const glm::mat4& getViewMatrix() noexcept;
    [[nodiscard]] const glm::mat4& getViewInverseMatrix() noexcept;

    [[nodiscard]] const glm::mat4& getProjectionMatrix() noexcept;
    [[nodiscard]] const glm::mat4& getProjectionInverseMatrix() noexcept;

    // Setters

    void setVelocity(const glm::vec3& velocity) noexcept { _velocity = velocity; }
    void setSpeed(const float speed) noexcept { _speed = speed; }

    void addFOV(const float delta) noexcept {
        setFOV(_fov + delta);
    }

    void addYaw(const float delta) noexcept {

        _orientation = glm::normalize(
            glm::angleAxis(
                glm::radians(delta),
                glm::vec3(0.0f, 0.0f, 1.0f)
            ) * _orientation
        );

        updateRotationBasis();
    }

    void addPitch(const float delta) noexcept {

        _orientation = glm::normalize(
            _orientation * glm::angleAxis(
                glm::radians(delta),
                glm::vec3(1.0f, 0.0f, 0.0f)
            )
        );

        updateRotationBasis();
    }

    void addRoll(const float delta) noexcept {

        _orientation = glm::normalize(
            glm::angleAxis(
                glm::radians(delta),
                _forward
            ) * _orientation
        );

        updateRotationBasis();
    }

    void addSpeed(const float delta) noexcept {
        _speed += delta;
    }

    void addVelocity(const glm::vec3& delta) noexcept {
        _velocity += delta;
    }

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

private:
    glm::vec3 _position = {3.0f, 0.0f, 3.0f};
    glm::vec3 _target   = {0.0f, 0.0f, 0.0f};

    // Rotation
    glm::vec3 _forward = {1.0f,  0.0f, 0.0f};
    glm::vec3 _right   = {0.0f, -1.0f, 0.0f};
    glm::vec3 _up      = {0.0f,  0.0f, 1.0f};

    glm::quat _orientation = {1.0f, 0.0f, 0.0f, 0.0};

    // Movement
    glm::vec3 _velocity = {0.0f, 0.0f, 0.0f};

    float _speed = 5.0f;

    float _rotationSensitivity = 0.2f;

    // Projection
    static constexpr float MIN_FOV = 30.0f;
    static constexpr float MAX_FOV = 140.0f;

    float _aspectRatio = 1.0f;
    float _fov         = 70.0f;
    float _nearPlane   = 0.01f;
    float _farPlane    = 100.0f;

    // Matrices
    glm::mat4 _viewMatrix        = {1.0f};
    glm::mat4 _viewInverseMatrix = {1.0f};

    glm::mat4 _projectionMatrix        = {0.0f};
    glm::mat4 _projectionInverseMatrix = {0.0f};

    bool _viewMatrixDirty  = true;
    bool _viewInverseValid = false;

    bool _projectionMatrixDirty  = true;
    bool _projectionInverseValid = false;
};
