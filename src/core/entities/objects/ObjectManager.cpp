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

void ObjectManager::createObjects() {
#if MULTITHREADED_OBJECTS_LOAD

    // Multithreaded objects loading (models, textures) using a thread pool
    unsigned int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4; // Fail-safe

    ThreadPool threadPool{numThreads * 2};

    // Load models
    std::vector<std::string> modelPaths{};
    for (const auto& objectDescriptor : _objectDescriptors) {
        modelPaths.push_back(objectDescriptor.modelPath);
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    loadModelsAsync(threadPool, modelPaths);

    auto endTime = std::chrono::high_resolution_clock::now();

    auto loadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    Logger::info("Loaded models in " + std::to_string(loadDuration) + "ms");

    // Load textures
    std::vector<std::string> texturePaths{};
    for (const auto& model : _models | std::views::values) {
        if (model->texturePaths.empty()) continue;
        for (const auto& texturePath : model->texturePaths) {
            if (!texturePath.empty()) {
                texturePaths.push_back(texturePath);
            }
        }
    }

    startTime = std::chrono::high_resolution_clock::now();

    loadTexturesAsync(threadPool, texturePaths);

    endTime = std::chrono::high_resolution_clock::now();

    loadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    Logger::info("Loaded textures in " + std::to_string(loadDuration) + "ms");

    // Create objects
    std::vector<std::future<std::unique_ptr<Object>>> objectFutures;

    for (const auto& objectDescriptor : _objectDescriptors) {
        objectFutures.push_back(
            threadPool.enqueue([this, objectDescriptor]() -> std::unique_ptr<Object> {
                if (!_models.contains(objectDescriptor.modelPath)) return nullptr;

                // Retrieve previously loaded model
                const Model* model = _models.at(objectDescriptor.modelPath);
                if (!model) return nullptr;

                // Retrieve previously loaded textures for this model
                TexturesMap modelTextures{};

                for (const auto& texturePath : model->texturePaths) {
                    if (_textures.contains(texturePath)) {
                        modelTextures[texturePath] = _textures.at(texturePath);
                    }
                }

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

#else

    std::string errorMessage;

    for (const auto& [modelPath, position, rotation, scale] : _objectDescriptors) {
        // Load model
        const Model* model = _modelManager->load(modelPath, errorMessage);
        if (!model) Logger::warning(errorMessage);

        // Load textures and map them to their respective path
        for (const auto& texturePath : model->texturePaths) {
            _textures[texturePath] = _imageManager->load(texturePath, errorMessage, Object::MIPMAPS_ENABLED);
            if (!_textures[texturePath]) Logger::warning(errorMessage);
        }

        // Create the object and push its pointer to the vector
        _objects.push_back(std::make_unique<Object>());

        _objects.back()->create(model, position, rotation, scale);
    }

#endif
}

#if MULTITHREADED_OBJECTS_LOAD

void ObjectManager::loadModelsAsync(ThreadPool& threadPool, const std::vector<std::string>& modelPaths) {
    std::unordered_map<std::string, std::shared_future<const Model*>> modelFutures;

    for (const auto& modelPath : modelPaths) {
        if (modelPath.empty()) continue;

        if (!modelFutures.contains(modelPath)) {
            modelFutures[modelPath] = threadPool.enqueue([this, modelPath] {
                std::string errorMessage;

                const Model* model = _modelManager->load(modelPath, errorMessage);
                if (!model) Logger::error(errorMessage);

                return model;
            }).share();
        }
    }

    for (auto& [modelPath, modelFuture] : modelFutures) {
        _models.emplace(modelPath, modelFuture.get());
    }
}

void ObjectManager::loadTexturesAsync(ThreadPool& threadPool, const std::vector<std::string>& texturePaths) {
    std::unordered_map<std::string, std::shared_future<const Image*>> textureFutures;

    for (const auto& texturePath : texturePaths) {
        if (texturePath.empty()) continue;

        if (!textureFutures.contains(texturePath)) {
            textureFutures[texturePath] = threadPool.enqueue([this, texturePath] {
                std::string errorMessage;

                const Image* texture = _imageManager->load(texturePath, errorMessage, Object::MIPMAPS_ENABLED);
                if (!texture) Logger::warning(errorMessage);

                return texture;
            }).share();
        }
    }

    for (auto& [texturePath, textureFuture] : textureFutures) {
        _textures.emplace(texturePath, textureFuture.get());
    }
}

#endif
