#pragma once
#ifndef NOBLEENGINE_OBJECTMANAGER_H
#define NOBLEENGINE_OBJECTMANAGER_H

#include "Object.h"
#include "core/common/Types.h"

#include "core/resources/images/ImageManager.h"
#include "core/resources/models/ModelManager.h"

class ObjectManager {
public:
    explicit ObjectManager(ModelManager* modelManager, ImageManager* imageManager)
        : _modelManager(modelManager), _imageManager(imageManager) {}

    ~ObjectManager() = default;

    ObjectManager(const ObjectManager&)            = delete;
    ObjectManager& operator=(const ObjectManager&) = delete;

    ObjectManager(ObjectManager&&)            = delete;
    ObjectManager& operator=(ObjectManager&&) = delete;

    Object* createObject(
        const std::string& modelPath,
        glm::vec3          position,
        glm::vec3          rotation,
        glm::vec3          scale
    );

    [[nodiscard]] const objects_vector& getObjects() const noexcept { return _objects; }

private:
    ModelManager* _modelManager = nullptr;
    ImageManager* _imageManager = nullptr;

    objects_vector _objects{};
};

#endif // NOBLEENGINE_OBJECTMANAGER_H
