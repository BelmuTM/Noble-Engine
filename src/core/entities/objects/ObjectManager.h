#pragma once
#ifndef NOBLEENGINE_OBJECTMANAGER_H
#define NOBLEENGINE_OBJECTMANAGER_H

#include "Object.h"

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

    void createObjects();

#if MULTITHREADED_OBJECTS_LOAD

    void loadModelsAsync(ThreadPool& threadPool, const std::vector<std::string>& modelPaths) const;

    void loadTexturesAsync(ThreadPool& threadPool, const std::vector<std::string>& texturePaths) const;

#endif

    [[nodiscard]] const ObjectsVector& getObjects() const noexcept { return _objects; }

private:
    AssetManager& _assetManager;

    struct ObjectDescriptor {
        std::string modelPath;
        glm::vec3   position;
        glm::vec3   rotation;
        glm::vec3   scale;
    };

    std::vector<ObjectDescriptor> _objectDescriptors{};

    ObjectsVector _objects{};
};

#endif // NOBLEENGINE_OBJECTMANAGER_H
