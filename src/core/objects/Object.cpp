#include "Object.h"

#include <glm/ext/matrix_transform.hpp>

void Object::create(
    const std::string& modelPath,
    const std::string& texturePath,
    const glm::vec3    position,
    const glm::vec3    rotation,
    const glm::vec3    scale
) {
    _modelPath   = modelPath;
    _texturePath = texturePath;

    _position = position;
    _rotation = rotation;
    _scale    = scale;
}

glm::mat4 Object::getModelMatrix() const {
    auto model = glm::mat4(1.0f);
    model = glm::translate(model, _position);
    model = glm::rotate(model, glm::radians(_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, _scale);
    return model;
}
