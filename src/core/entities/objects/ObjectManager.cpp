#include "ObjectManager.h"

#include "core/debug/Logger.h"

#include <memory>
#include <ranges>

#define MULTITHREADED_OBJECTS_LOAD 1

void ObjectManager::addObject(
    const std::string& modelPath,
    const glm::vec3    position,
    const glm::vec3    rotation,
    const glm::vec3    scale
) {
    _objectDescriptors.emplace_back(modelPath, position, rotation, scale);
}

void ObjectManager::createObjects() {
    const auto startTime = std::chrono::high_resolution_clock::now();

#if MULTITHREADED_OBJECTS_LOAD == 1

    // Multithreaded objects loading (models, textures) using a thread pool
    unsigned int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4; // Fail-safe

    ThreadPool threadPool{numThreads * 2};

    // Load models
    std::vector<std::string> modelPaths{};
    for (const auto& objectDescriptor : _objectDescriptors) {
        modelPaths.push_back(objectDescriptor.modelPath);
    }

    const std::unordered_map<std::string, const Model*>& models = loadModelsAsync(threadPool, modelPaths);

    // Load textures
    std::vector<std::string> texturePaths{};
    for (const auto& model : models | std::views::values) {
        for (const auto& texturePath : model->texturePaths) {
            if (!texturePath.empty()) {
                texturePaths.push_back(texturePath);
            }
        }
    }

    const Object::TexturesMap& textures = loadTexturesAsync(threadPool, texturePaths);

    // Create objects
    std::vector<std::future<std::unique_ptr<Object>>> objectFutures;

    for (const auto& objectDescriptor : _objectDescriptors) {
        objectFutures.push_back(
            threadPool.enqueue([objectDescriptor, &models, &textures]() -> std::unique_ptr<Object> {
                if (!models.contains(objectDescriptor.modelPath)) return nullptr;

                // Retrieve previously loaded model
                const Model* model = models.at(objectDescriptor.modelPath);
                if (!model) return nullptr;

                // Retrieve previously loaded textures for this model
                Object::TexturesMap modelTextures;
                for (const auto& texturePath : model->texturePaths) {
                    if (textures.contains(texturePath)) {
                        modelTextures[texturePath] = textures.at(texturePath);
                    }
                }

                auto object = std::make_unique<Object>();

                object->create(
                    model, modelTextures, objectDescriptor.position, objectDescriptor.rotation, objectDescriptor.scale
                );

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

        // Gather texture paths
        std::vector<std::string> texturePaths{};

        for (const auto& mesh : model->meshes) {
            const Material& material = mesh.getMaterial();

            texturePaths.push_back(material.albedoPath);
            texturePaths.push_back(material.normalPath);
            texturePaths.push_back(material.specularPath);
        }

        // Load textures and map them to their respective path
        Object::TexturesMap textures{};

        for (const auto& texturePath : texturePaths) {
            textures[texturePath] = _imageManager->load(texturePath, errorMessage);
            if (!textures[texturePath]) Logger::warning(errorMessage);
        }

        // Create the object and push its pointer to the vector
        _objects.push_back(std::make_unique<Object>());

        _objects.back()->create(
            model, textures, position, rotation, scale
        );
    }

#endif

    const auto endTime = std::chrono::high_resolution_clock::now();

    const auto loadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    Logger::info("Loaded objects in " + std::to_string(loadDuration) + "ms");
}

std::unordered_map<std::string, const Model*> ObjectManager::loadModelsAsync(
    ThreadPool& threadPool, const std::vector<std::string>& modelPaths
) const {
    std::unordered_map<std::string, std::shared_future<Model*>> modelFutures;

    for (const auto& modelPath : modelPaths) {
        if (modelPath.empty()) continue;

        if (!modelFutures.contains(modelPath)) {
            modelFutures[modelPath] = threadPool.enqueue([this, modelPath] {
                std::string errorMessage;

                Model* model = _modelManager->load(modelPath, errorMessage);
                if (!model) Logger::warning(errorMessage);

                if (model) {
                    for (const auto& mesh : model->meshes) {
                        const auto& material = mesh.getMaterial();

                        model->texturePaths.push_back(material.albedoPath);
                        model->texturePaths.push_back(material.normalPath);
                        model->texturePaths.push_back(material.specularPath);
                    }
                }

                return model;
            }).share();
        }
    }

    std::unordered_map<std::string, const Model*> loadedModels{};

    for (auto& [modelPath, modelFuture] : modelFutures) {
        loadedModels.emplace(modelPath, modelFuture.get());
    }

    return loadedModels;
}

std::unordered_map<std::string, const Image*> ObjectManager::loadTexturesAsync(
    ThreadPool& threadPool, const std::vector<std::string>& texturePaths
) const {
    std::unordered_map<std::string, std::shared_future<const Image*>> textureFutures;

    for (const auto& texturePath : texturePaths) {
        if (texturePath.empty()) continue;

        if (!textureFutures.contains(texturePath)) {
            textureFutures[texturePath] = threadPool.enqueue([this, texturePath] {
                std::string errorMessage;

                const Image* texture = _imageManager->load(texturePath, errorMessage);
                if (!texture) Logger::warning(errorMessage);

                return texture;
            }).share();
        }
    }

    std::unordered_map<std::string, const Image*> loadedTextures{};

    for (auto& [texturePath, textureFuture] : textureFutures) {
        loadedTextures.emplace(texturePath, textureFuture.get());
    }

    return loadedTextures;
}
