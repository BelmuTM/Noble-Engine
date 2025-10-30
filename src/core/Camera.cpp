#include "Camera.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

void Camera::move(const glm::vec3& delta) {
    _position += delta;
    _target   += delta;
    updateViewMatrix();
}

glm::mat4 Camera::getProjectionMatrix(const float aspectRatio) const {
    return glm::perspective(glm::radians(_fov), aspectRatio, _nearPlane, _farPlane);
}

void Camera::updateViewMatrix() {
    _viewMatrix = glm::lookAt(_position, _target, _up);
}
