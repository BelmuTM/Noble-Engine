#pragma once
#ifndef NOBLEENGINE_OBJECT_H
#define NOBLEENGINE_OBJECT_H

#include "core/resources/images/Image.h"
#include "core/resources/models/Model.h"

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

struct alignas(16) ObjectDataGPU {
    glm::mat4 model;
    glm::mat4 normal;
};

class Object {
public:
    using TexturesMap = std::unordered_map<std::string, const Image*>;

    Object()  = default;
    ~Object() = default;

    Object(const Object&)            = delete;
    Object& operator=(const Object&) = delete;

    Object(Object&&)            noexcept = default;
    Object& operator=(Object&&) noexcept = default;

    void create(
        const Model*        model,
        const TexturesMap& textures,
        glm::vec3           position = {0.0f, 0.0f, 0.0f},
        glm::vec3           rotation = {0.0f, 0.0f, 0.0f},
        glm::vec3           scale    = {1.0f, 1.0f, 1.0f}
    );

    void updateMatrices();

    [[nodiscard]] const Model& getModel() const noexcept { return *_model; }

    [[nodiscard]] TexturesMap& getTexturesMap() noexcept { return _textures; }

    [[nodiscard]] const Image* getTexture(const std::string& path) const {
        return _textures.contains(path) ? _textures.at(path) : nullptr;
    }

    [[nodiscard]] glm::mat4 getModelMatrix() const noexcept { return _modelMatrix; }
    [[nodiscard]] glm::mat4 getNormalMatrix() const noexcept { return _normalMatrix; }

private:
    const Model* _model =  nullptr;

    TexturesMap _textures{};

    glm::vec3 _position = {0.0f, 0.0f, 0.0f};
    glm::vec3 _rotation = {0.0f, 0.0f, 0.0f};
    glm::vec3 _scale    = {1.0f, 1.0f, 1.0f};

    glm::vec3 _lastPosition{};
    glm::vec3 _lastRotation{};
    glm::vec3 _lastScale{};

    glm::mat4 _modelMatrix{};
    glm::mat4 _normalMatrix{};
};

#endif //NOBLEENGINE_OBJECT_H