#pragma once
#ifndef NOBLEENGINE_OBJECT_H
#define NOBLEENGINE_OBJECT_H

#include <string>

#include <glm/glm.hpp>

class Object {
public:
    Object()  = default;
    ~Object() = default;

    Object(const Object&)            noexcept = default;
    Object& operator=(const Object&) noexcept = default;

    Object(Object&&)            = delete;
    Object& operator=(Object&&) = delete;

    void create(
        const std::string& modelPath,
        const std::string& texturePath,
        glm::vec3          position = {0.0f, 0.0f, 0.0f},
        glm::vec3          rotation = {0.0f, 0.0f, 0.0f},
        glm::vec3          scale    = {1.0f, 1.0f, 1.0f}
    );

    [[nodiscard]] glm::mat4 getModelMatrix() const;

    [[nodiscard]] std::string getModelPath() const { return _modelPath; }

    [[nodiscard]] std::string getTexturePath() const { return _texturePath; }

private:
    glm::vec3 _position = {0.0f, 0.0f, 0.0f};
    glm::vec3 _rotation = {0.0f, 0.0f, 0.0f};
    glm::vec3 _scale    = {1.0f, 1.0f, 1.0f};

    std::string _modelPath;
    std::string _texturePath;
};

#endif //NOBLEENGINE_OBJECT_H