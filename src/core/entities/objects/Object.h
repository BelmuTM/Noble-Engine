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
    glm::vec3 debugColor = {1.0f, 1.0f, 1.0f};
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

    // Getters

    [[nodiscard]] const Model& getModel() const noexcept { return *_model; }

    [[nodiscard]] const glm::vec3& getPosition() const noexcept { return _position; }
    [[nodiscard]] const glm::vec3& getRotation() const noexcept { return _rotation; }
    [[nodiscard]] const glm::vec3& getScale() const noexcept { return _scale; }

    [[nodiscard]] const glm::mat4& getModelMatrix() const noexcept { return _modelMatrix; }
    [[nodiscard]] const glm::mat4& getNormalMatrix() const noexcept { return _normalMatrix; }

    [[nodiscard]] std::uint32_t getVersion() const noexcept { return _version; }

    // Setters

    void setModel(const Model* model) noexcept { _model = model; }

    void setPosition(const glm::vec3& position) noexcept {
        if (_position == position) return;

        _position = position;
        _stale    = true;
    }

    void setRotation(const glm::vec3& rotation) noexcept {
        if (_rotation == rotation) return;

        _rotation = rotation;
        _stale    = true;
    }

    void setScale(const glm::vec3& scale) noexcept {
        if (_scale == scale) return;

        _scale = scale;
        _stale = true;
    }

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

    bool          _stale   = true;
    std::uint32_t _version = 0;
};
