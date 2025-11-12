#include "Object.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

void Object::create(
    const Model*        model,
    const textures_map& textures,
    const glm::vec3     position,
    const glm::vec3     rotation,
    const glm::vec3     scale
) {
    _model    = model;
    _textures = textures;
    _position = position;
    _rotation = rotation;
    _scale    = scale;

    updateMatrices();
}

void Object::updateMatrices() {
    if (_lastPosition == _position &&
        _lastRotation == _rotation &&
        _lastScale    == _scale) {
        return;
    }

    _lastPosition = _position;
    _lastRotation = _rotation;
    _lastScale    = _scale;

    const glm::mat4 translation = glm::translate(glm::mat4(1.0f), _position);
    const glm::mat4 rotation    = glm::eulerAngleXYZ(
        glm::radians(_rotation.x),
        glm::radians(_rotation.y),
        glm::radians(_rotation.z)
    );
    const glm::mat4 scaling     = glm::scale(glm::mat4(1.0f), _scale);

    _modelMatrix  = translation * rotation * scaling;
    _normalMatrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(_modelMatrix))));
}
