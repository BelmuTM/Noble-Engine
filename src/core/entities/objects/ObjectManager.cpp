#include "ObjectManager.h"

#include "core/debug/Logger.h"

#include <memory>
#include <ranges>

void ObjectManager::addObject(
    const std::string& modelPath,
    const glm::vec3    position,
    const glm::vec3    rotation,
    const glm::vec3    scale
) {
    _objectDescriptors.emplace_back(modelPath, position, rotation, scale);
}

void ObjectManager::addScene(const Scene& scene) {
    for (const auto& [modelPath, position, rotation, scale] : scene.getObjects()) {
        _objectDescriptors.emplace_back(modelPath, position, rotation, scale);
    }
}

void ObjectManager::createObjects() {

    // Multithreaded objects loading (models, textures) using a thread pool
    unsigned int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4; // Fail-safe

    ThreadPool threadPool{numThreads};

    // Load models
    _modelPaths.clear();

    for (const auto& objectDescriptor : _objectDescriptors) {
        _modelPaths.push_back(objectDescriptor.modelPath);
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    _assetManager.loadModelsAsync(threadPool, _modelPaths);

    auto loadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();

    Logger::info("Loaded models in " + std::to_string(loadDuration) + "ms");

    // Load textures
    _texturePaths.clear();

    for (const auto& model : _assetManager.getModels() | std::views::values) {
        if (model->resource->texturePaths.empty()) continue;

        for (const auto& texturePath : model->resource->texturePaths) {
            if (!texturePath.empty()) {
                _texturePaths.push_back(texturePath);
            }
        }
    }

    startTime = std::chrono::high_resolution_clock::now();

    _assetManager.loadTexturesAsync(threadPool, _texturePaths);

    loadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();

    Logger::info("Loaded textures in " + std::to_string(loadDuration) + "ms");

    // Create objects
    std::vector<std::future<std::unique_ptr<Object>>> objectFutures;

    for (const auto& [modelPath, position, rotation, scale] : _objectDescriptors) {
        const Model* model = _assetManager.getModelManager().get(modelPath);

        if (!model) {
            Logger::error("Failed to create object: model not ready: " + modelPath);
            continue;
        }

        auto object = std::make_unique<Object>();
        object->create(model, position, rotation, scale);
        _objects.push_back(std::move(object));
    }

    for (auto& objectFuture : objectFutures) {
        if (objectFuture.valid()) {
            if (auto object = objectFuture.get())
                _objects.push_back(std::move(object));
        }
    }
}
