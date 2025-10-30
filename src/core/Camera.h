#pragma once
#ifndef NOBLEENGINE_CAMERA_H
#define NOBLEENGINE_CAMERA_H

#include <glm/glm.hpp>

class Camera {
public:
    Camera()  = default;
    ~Camera() = default;

    Camera(const Camera&)            noexcept = default;
    Camera& operator=(const Camera&) noexcept = default;

    Camera(Camera&&)            = delete;
    Camera& operator=(Camera&&) = delete;

    void move(const glm::vec3& delta);

    void setFOV(const float fov) { _fov = fov; }

    [[nodiscard]] glm::mat4 getViewMatrix() const { return _viewMatrix; }

    [[nodiscard]] glm::mat4 getProjectionMatrix(float aspectRatio) const;

private:
    float _fov = 45.0f;

    float _nearPlane = 0.1f;
    float _farPlane  = 10.0f;

    glm::vec3 _position = {3.0f, 3.0f, 2.0f};
    glm::vec3 _target   = {0.0f, 0.0f, 0.0f};
    glm::vec3 _up       = {0.0f, 0.0f, 1.0f};

    glm::mat4 _viewMatrix = {1.0f};

    void updateViewMatrix();
};

#endif //NOBLEENGINE_CAMERA_H
