#include "Camera.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

void Camera::update(const double deltaTime) {
    if (_controller) {
        _controller->update();
    }

    glm::vec3 forward = glm::normalize(glm::vec3(_front.x, _front.y, 0.0f)); // ignore Z if you want horizontal movement
    glm::vec3 right   = glm::normalize(glm::cross(forward, _up));

    glm::vec3 worldVelocity = _velocity.x * forward + _velocity.y * right + _velocity.z * _up;

    move(worldVelocity * _speed * static_cast<float>(deltaTime));

    updateViewMatrix();
}

glm::mat4 Camera::getProjectionMatrix(const float aspectRatio) const {
    return glm::perspective(glm::radians(_fov), aspectRatio, _nearPlane, _farPlane);
}

void Camera::updateViewMatrix() {
    //_front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    //_front.y = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    //_front.z = sin(glm::radians(_pitch));
    //_front = glm::normalize(_front);

    _viewMatrix = glm::lookAt(_position, _position + _front, _up);
}
