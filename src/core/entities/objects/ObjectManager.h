#pragma once
#ifndef NOBLEENGINE_OBJECTMANAGER_H
#define NOBLEENGINE_OBJECTMANAGER_H

#include "Object.h"
#include "core/common/Types.h"
#include "core/concurrency/ThreadPool.h"

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

    void addObject(
        const std::string& modelPath,
        glm::vec3          position = {0.0f, 0.0f, 0.0f},
        glm::vec3          rotation = {0.0f, 0.0f, 0.0f},
        glm::vec3          scale    = {1.0f, 1.0f, 1.0f}
    );

    void createObjects();

    [[nodiscard]] std::unordered_map<std::string, const Model*> loadModelsAsync(
        ThreadPool& threadPool, const std::vector<std::string>& modelPaths
    ) const;

    [[nodiscard]] Object::TexturesMap loadTexturesAsync(
        ThreadPool& threadPool, const std::vector<std::string>& texturePaths
    ) const;

    [[nodiscard]] const ObjectsVector& getObjects() const noexcept { return _objects; }

private:
    struct ObjectDescriptor {
        std::string modelPath;
        glm::vec3   position;
        glm::vec3   rotation;
        glm::vec3   scale;
    };

    ModelManager* _modelManager = nullptr;
    ImageManager* _imageManager = nullptr;

    std::vector<ObjectDescriptor> _objectDescriptors{};

    ObjectsVector _objects{};
};

#endif // NOBLEENGINE_OBJECTMANAGER_H
