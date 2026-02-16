#pragma once

#include "core/resources/models/Model.h"

#include <glm/glm.hpp>

struct ObjectDescriptor {
    std::string modelPath;
    glm::vec3   position;
    glm::vec3   rotation;
    glm::vec3   scale;
};

struct alignas(16) ObjectDataGPU {
    glm::mat4 modelMatrix;
    glm::mat4 normalMatrix;
};

class Object {
public:
    Object()  = default;
    ~Object() = default;

    Object(const Object&)            = delete;
    Object& operator=(const Object&) = delete;

    Object(Object&&)            noexcept = default;
    Object& operator=(Object&&) noexcept = default;

    void create(
        const Model* model,
        glm::vec3    position = {0.0f, 0.0f, 0.0f},
        glm::vec3    rotation = {0.0f, 0.0f, 0.0f},
        glm::vec3    scale    = {1.0f, 1.0f, 1.0f}
    );

    void updateMatrices();

    [[nodiscard]] const Model& getModel() const noexcept { return *_model; }

    [[nodiscard]] glm::mat4 getModelMatrix() const noexcept { return _modelMatrix; }
    [[nodiscard]] glm::mat4 getNormalMatrix() const noexcept { return _normalMatrix; }

private:
    const Model* _model =  nullptr;

    glm::vec3 _position = {0.0f, 0.0f, 0.0f};
    glm::vec3 _rotation = {0.0f, 0.0f, 0.0f};
    glm::vec3 _scale    = {1.0f, 1.0f, 1.0f};

    glm::vec3 _lastPosition{};
    glm::vec3 _lastRotation{};
    glm::vec3 _lastScale{};

    glm::mat4 _modelMatrix{};
    glm::mat4 _normalMatrix{};
};
