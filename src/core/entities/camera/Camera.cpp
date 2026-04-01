#include "Camera.h"

#include <cmath>

const glm::mat4& Camera::getViewMatrix() noexcept {
    if (_viewMatrixDirty) {
        _viewMatrix       = glm::lookAt(_position, _position + _forward, _up);
        _viewMatrixDirty  = false;
        _viewInverseValid = false;
    }

    return _viewMatrix;
}

const glm::mat4& Camera::getViewInverseMatrix() noexcept {
    if (!_viewInverseValid) {
        _viewInverseMatrix = glm::inverse(getViewMatrix());
        _viewInverseValid  = true;
    }

    return _viewInverseMatrix;
}

const glm::mat4& Camera::getProjectionMatrix() noexcept {
    if (_projectionMatrixDirty) {
        const float f_stops = 1.0f / std::tan(glm::radians(_fov) * 0.5f);

        _projectionMatrix[0][0] = f_stops / _aspectRatio;
        _projectionMatrix[1][1] = f_stops;

        _projectionMatrix[2][2] = 0.0f;
        _projectionMatrix[2][3] = -1.0f;
        _projectionMatrix[3][2] = _nearPlane;

        _projectionMatrixDirty  = false;
        _projectionInverseValid = false;
    }

    return _projectionMatrix;
}

const glm::mat4& Camera::getProjectionInverseMatrix() noexcept {
    if (!_projectionInverseValid) {
        _projectionInverseMatrix = glm::inverse(getProjectionMatrix());
        _projectionInverseValid  = true;
    }

    return _projectionInverseMatrix;
}

void Camera::update(const float deltaTime) {
    const glm::vec3 worldVelocity = toWorldSpace(_velocity);

    move(worldVelocity * deltaTime);
}

glm::vec3 Camera::toWorldSpace(const glm::vec3 vector) const {
    const glm::vec3 forward = glm::normalize(glm::vec3(_forward.x, _forward.y, 0.0f));
    const glm::vec3 right   = glm::normalize(glm::cross(forward, _up));

    return vector.x * forward + vector.y * right + vector.z * _up;
}
void Camera::updateRotation() {
    _forward.x = static_cast<float>(cos(glm::radians(_yaw)) * cos(glm::radians(_pitch)));
    _forward.y = static_cast<float>(sin(glm::radians(_yaw)) * cos(glm::radians(_pitch)));
    _forward.z = static_cast<float>(sin(glm::radians(_pitch)));
    _forward   = glm::normalize(_forward);

    _viewMatrixDirty = true;
}

