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

    static void loadMaterial_OBJ(Mesh& mesh, const tinyobj::material_t& material);
    static void loadMaterial_glTF(
        Mesh&                                 mesh,
        const std::string&                    modelName,
        const tinygltf::Material&             material,
        const std::vector<tinygltf::Texture>& textures,
        const std::vector<tinygltf::Image>&   images
    );

    [[nodiscard]] static bool load_OBJ(Model& model, const std::string& path, std::string& errorMessage);
    [[nodiscard]] static bool load_glTF(Model& model, const std::string& path, std::string& errorMessage);

    [[nodiscard]] const Model* load(const std::string& path, std::string& errorMessage);

private:
    std::unordered_map<std::string, std::unique_ptr<Model>> _cache{};

    std::mutex _mutex{};
};

#endif // NOBLEENGINE_MODELMANAGER_H
