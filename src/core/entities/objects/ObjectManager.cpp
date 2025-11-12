#include "ObjectManager.h"

#include "core/debug/Logger.h"

#include <memory>

Object* ObjectManager::createObject(
    const std::string& modelPath,
    const glm::vec3    position,
    const glm::vec3    rotation,
    const glm::vec3    scale
) {
    std::string errorMessage;

    // Load model
    const Model* model = _modelManager->load(modelPath, errorMessage);
    if (!model) Logger::warning(errorMessage);

    // Gather texture paths
    std::vector<std::string> texturePaths{};

    for (const auto& mesh : model->meshes) {
        const Material& material = mesh.getMaterial();

        texturePaths.push_back(material.albedoPath);
    }

    // Load textures and map them to their respective path
    Object::textures_map textures{};

    for (const auto& texturePath : texturePaths) {
        textures[texturePath] = _imageManager->load(texturePath, errorMessage);
    }

    // Create the object and push its pointer to the vector
    _objects.push_back(std::make_unique<Object>());
    _objects.back()->create(model, textures, position, rotation, scale);

    // Return object pointer
    return _objects.back().get();
}
