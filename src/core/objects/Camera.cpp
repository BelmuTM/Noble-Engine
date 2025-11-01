#include "Camera.h"

void Camera::updateRotation() {
    _forward.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    _forward.y = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    _forward.z = sin(glm::radians(_pitch));
    _forward   = glm::normalize(_forward);
    updateViewMatrix();
}

void Camera::update(const double deltaTime) {
    if (_controller) {
        _controller->update();
    }

    const glm::vec3 worldVelocity = toWorldSpace(_velocity);
    move(worldVelocity * _speed * static_cast<float>(deltaTime));

    updateViewMatrix();
}

void Camera::updateViewMatrix() {
    _viewMatrix = glm::lookAt(_position, _position + _forward, _up);
}

glm::vec3 Camera::toWorldSpace(const glm::vec3 vector) const {
    const glm::vec3 forward = glm::normalize(glm::vec3(_forward.x, _forward.y, 0.0f));
    const glm::vec3 right   = glm::normalize(glm::cross(forward, _up));

    return vector.x * forward + vector.y * right + vector.z * _up;
}
