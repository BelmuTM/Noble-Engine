#pragma once

#include "Model.h"

#include "core/debug/ErrorHandling.h"
#include "core/resources/AsyncResourceManager.h"

#include "libraries/tinygltfUsage.h"
#include "libraries/tinyobjloaderUsage.h"

#include <future>
#include <memory>

class ModelManager : public AsyncResourceManager<Model> {
public:
    ModelManager()  = default;
    ~ModelManager() = default;

    ModelManager(const ModelManager&)            = delete;
    ModelManager& operator=(const ModelManager&) = delete;

    ModelManager(ModelManager&&)            = delete;
    ModelManager& operator=(ModelManager&&) = delete;

    ResourceHandlePointer load(const std::string& path);

    Expected<const Model*> loadBlocking(const std::string& path);

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

    [[nodiscard]] static Expected<void> load_OBJ(Model& model, const std::string& path);

    [[nodiscard]] static Expected<void> load_glTF(Model& model, const std::string& path, const std::string& extension);

private:
    static Mesh processMesh_OBJ(
        const std::string&                   modelName,
        const tinyobj::attrib_t&             attributes,
        const std::vector<tinyobj::shape_t>& shapes,
        const tinyobj::material_t&           material,
        int                                  targetMaterialIndex
    );

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
};
