#pragma once
#ifndef NOBLEENGINE_MODELMANAGER_H
#define NOBLEENGINE_MODELMANAGER_H

#include "Model.h"

#include "core/common/libraries/tinygltfUsage.h"
#include "core/common/libraries/tinyobjloaderUsage.h"

#include <mutex>
#include <unordered_map>

class ModelManager {
public:
    ModelManager()  = default;
    ~ModelManager() = default;

    ModelManager(const ModelManager&)            = delete;
    ModelManager& operator=(const ModelManager&) = delete;

    ModelManager(ModelManager&&)            = delete;
    ModelManager& operator=(ModelManager&&) = delete;

    [[nodiscard]] const Model* load(const std::string& path, std::string& errorMessage);

    static void loadMaterial_OBJ(
        Mesh&                      mesh,
        const std::string&         modelName,
        const tinyobj::material_t& material
    );

    static void loadMaterial_glTF(
        Mesh&                                 mesh,
        const std::string&                    modelName,
        const tinygltf::Material&             material,
        const std::vector<tinygltf::Texture>& textures,
        const std::vector<tinygltf::Image>&   images
    );

    [[nodiscard]] static bool load_OBJ(Model& model, const std::string& path, std::string& errorMessage);
    [[nodiscard]] static bool load_glTF(Model& model, const std::string& path, std::string& errorMessage);

private:
    static void processMeshPrimitives_glTF(
        Mesh&                      mesh,
        const tinygltf::Model&     glTFModel,
        const tinygltf::Primitive& primitive
    );

    static Mesh createMesh_glTF(
        const std::string&         modelName,
        const tinygltf::Model&     glTFModel,
        const tinygltf::Primitive& primitive
    );

    static void processNode_glTF(
        Model&                 model,
        const tinygltf::Model& glTFModel,
        const tinygltf::Node&  node,
        const glm::mat4&       parentTransform
    );

    std::unordered_map<std::string, std::unique_ptr<Model>> _cache{};

    std::mutex _mutex{};
};

#endif // NOBLEENGINE_MODELMANAGER_H
