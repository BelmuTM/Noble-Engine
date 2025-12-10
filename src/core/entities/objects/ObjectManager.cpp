#include "ObjectManager.h"

#include "core/concurrency/ThreadPool.h"
#include "core/debug/Logger.h"

#include <memory>

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

    std::vector<std::future<std::unique_ptr<Object>>> objectFutures{};

    for (const auto& objectDescriptor : _objectDescriptors) {
        objectFutures.push_back(
            threadPool.enqueue([this, &threadPool, objectDescriptor] {
                // Load model
                std::string errorMessageModelLoad;

                const Model* model = _modelManager->load(objectDescriptor.modelPath, errorMessageModelLoad);
                if (!model) Logger::warning(errorMessageModelLoad);

                // Gather texture paths
                std::vector<std::string> texturePaths{};

                for (const auto& mesh : model->meshes) {
                    const Material& material = mesh.getMaterial();

                    texturePaths.push_back(material.albedoPath);
                    texturePaths.push_back(material.normalPath);
                    texturePaths.push_back(material.specularPath);
                }

                // Load textures
                std::vector<std::future<const Image*>> textureFutures{};

                for (const auto& texturePath : texturePaths) {
                    textureFutures.push_back(
                        threadPool.enqueue([this, texturePath] {
                            std::string errorMessageTextureLoad;

                            const Image* texture = _imageManager->load(texturePath, errorMessageTextureLoad);
                            if (!texture) Logger::warning(errorMessageTextureLoad);

                            return texture;
                        })
                    );
                }

                // Map textures to their respective paths
                Object::TexturesMap textures{};

                for (size_t i = 0; i < texturePaths.size(); i++) {
                    textures[texturePaths[i]] = textureFutures[i].get();
                }

                // Create the object and push its pointer to the vector
                auto object = std::make_unique<Object>();

                object->create(
                    model, textures, objectDescriptor.position, objectDescriptor.rotation, objectDescriptor.scale
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
