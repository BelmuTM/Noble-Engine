#pragma once
#ifndef NOBLEENGINE_OBJECT_H
#define NOBLEENGINE_OBJECT_H

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

class Object {
public:
    Object()  = default;
    ~Object() = default;

    Object(const Object&)            noexcept = default;
    Object& operator=(const Object&) noexcept = default;

    Object(Object&&)            = delete;
    Object& operator=(Object&&) = delete;

    [[nodiscard]] glm::mat4 getModelMatrix() const;

private:
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
    glm::vec3 scale    = {1.0f, 1.0f, 1.0f};
};

#endif //NOBLEENGINE_OBJECT_H