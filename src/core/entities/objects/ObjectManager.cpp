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
#if MULTITHREADED_OBJECTS_LOAD

    // Multithreaded objects loading (models, textures) using a thread pool
    unsigned int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4; // Fail-safe

    ThreadPool threadPool{numThreads * 2};

    // Load models
    _modelPaths.clear();

    for (const auto& objectDescriptor : _objectDescriptors) {
        _modelPaths.push_back(objectDescriptor.modelPath);
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    _assetManager.loadModelsAsync(threadPool, _modelPaths);

    auto endTime = std::chrono::high_resolution_clock::now();

    auto loadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    Logger::info("Loaded models in " + std::to_string(loadDuration) + "ms");

    // Load textures
    _texturePaths.clear();

    for (const auto& model : _assetManager.getModels() | std::views::values) {
        if (model->texturePaths.empty()) continue;

        for (const auto& texturePath : model->texturePaths) {
            if (!texturePath.empty()) {
                _texturePaths.push_back(texturePath);
            }
        }
    }

    startTime = std::chrono::high_resolution_clock::now();

    _assetManager.loadTexturesAsync(threadPool, _texturePaths);

    endTime = std::chrono::high_resolution_clock::now();

    loadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    Logger::info("Loaded textures in " + std::to_string(loadDuration) + "ms");

    // Create objects
    std::vector<std::future<std::unique_ptr<Object>>> objectFutures;

    for (const auto& objectDescriptor : _objectDescriptors) {
        objectFutures.push_back(
            threadPool.enqueue([this, &objectDescriptor]() -> std::unique_ptr<Object> {
                // Retrieve previously loaded model
                const Model* model = _assetManager.getModelManager().get(objectDescriptor.modelPath);
                if (!model) return nullptr;

                auto object = std::make_unique<Object>();

                object->create(model, objectDescriptor.position, objectDescriptor.rotation, objectDescriptor.scale);

                return object;
            })
        );
    }

    for (auto& objectFuture : objectFutures) {
        if (auto object = objectFuture.get()) {
            _objects.push_back(std::move(object));
        }
    }

    Logger::debug("Created objects");

#else

    std::string errorMessage;

    for (const auto& [modelPath, position, rotation, scale] : _objectDescriptors) {
        // Load model
        const Model* model = _assetManager.getModelManager().loadBlocking(modelPath, errorMessage);
        if (!model) Logger::warning(errorMessage);

        // Load textures and map them to their respective path
        for (const auto& texturePath : model->texturePaths) {
            _assetManager.getTextures()[texturePath] =
                _assetManager.getImageManager().loadBlocking(texturePath, errorMessage, AssetManager::MIPMAPS_ENABLED);
            if (!_assetManager.getTextures()[texturePath]) Logger::warning(errorMessage);
        }

        // Create the object and push its pointer to the vector
        _objects.push_back(std::make_unique<Object>());

        _objects.back()->create(model, position, rotation, scale);
    }

#endif
}
