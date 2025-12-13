#pragma once
#ifndef NOBLEENGINE_OBJECTMANAGER_H
#define NOBLEENGINE_OBJECTMANAGER_H

#include "Object.h"

#include "core/concurrency/ThreadPool.h"

#include "core/resources/images/ImageManager.h"
#include "core/resources/models/ModelManager.h"

class ObjectManager {
public:
    using ObjectsVector = std::vector<std::unique_ptr<Object>>;
    using ModelsMap     = std::unordered_map<std::string, const Model*>;
    using TexturesMap   = std::unordered_map<std::string, const Image*>;

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

    void loadModelsAsync(ThreadPool& threadPool, const std::vector<std::string>& modelPaths);

    void loadTexturesAsync(ThreadPool& threadPool, const std::vector<std::string>& texturePaths);

    [[nodiscard]] const ObjectsVector& getObjects() const noexcept { return _objects; }

    [[nodiscard]] const TexturesMap& getTextures() const noexcept { return _textures; }

    [[nodiscard]] const Image* getTexture(const std::string& path) const {
        return _textures.contains(path) ? _textures.at(path) : nullptr;
    }

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

    ModelsMap   _models{};
    TexturesMap _textures{};
};

#endif // NOBLEENGINE_OBJECTMANAGER_H
