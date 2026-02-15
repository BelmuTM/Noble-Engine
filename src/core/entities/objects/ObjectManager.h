#pragma once
#ifndef NOBLEENGINE_OBJECTMANAGER_H
#define NOBLEENGINE_OBJECTMANAGER_H

#include "Object.h"

#include "core/entities/scenes/Scene.h"

#include "core/resources/AssetManager.h"

#include "core/concurrency/ThreadPool.h"

#define MULTITHREADED_OBJECTS_LOAD 1

class ObjectManager {
public:
    using ObjectsVector = std::vector<std::unique_ptr<Object>>;

    explicit ObjectManager(AssetManager& assetManager) : _assetManager(assetManager) {}

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

    void addScene(const Scene& scene);

    void createObjects();

    [[nodiscard]] const ObjectsVector& getObjects() const noexcept { return _objects; }

    [[nodiscard]] const std::vector<std::string>& getTexturePaths() const noexcept { return _texturePaths; }

private:
    AssetManager& _assetManager;

    std::vector<ObjectDescriptor> _objectDescriptors{};

    ObjectsVector _objects{};

    std::vector<std::string> _modelPaths{};
    std::vector<std::string> _texturePaths{};
};

#endif // NOBLEENGINE_OBJECTMANAGER_H
