#include "ModelManager.h"

#include "core/ResourceManager.h"
#include "core/debug/Logger.h"

#include <glm/gtc/type_ptr.hpp>

/*---------------------------------------*/
/*           .OBJ file loader            */
/*---------------------------------------*/

void ModelManager::loadMaterial_OBJ(Mesh& mesh, const tinyobj::material_t& material) {
    Material meshMaterial{};

    meshMaterial.name = material.name;

    meshMaterial.diffuse  = glm::make_vec3(material.diffuse);
    meshMaterial.specular = glm::make_vec3(material.specular);
    meshMaterial.emission = glm::make_vec3(material.emission);

    meshMaterial.albedoPath    = material.diffuse_texname;
    meshMaterial.normalPath    = material.normal_texname;
    meshMaterial.specularPath  = material.specular_texname;
    meshMaterial.roughnessPath = material.roughness_texname;
    meshMaterial.metallicPath  = material.metallic_texname;

    meshMaterial.ior       = material.ior;
    meshMaterial.metallic  = material.metallic;
    meshMaterial.roughness = material.roughness;

    mesh.setMaterial(meshMaterial);
}

bool ModelManager::load_OBJ(Model& model, const std::string& path, std::string& errorMessage) {
    Mesh mesh{};

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    tinyobj::attrib_t attributes;

    std::vector<tinyobj::shape_t>    shapes{};
    std::vector<tinyobj::material_t> materials{};

    const std::string fullPath = modelFilesPath + path;

    if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &errorMessage, fullPath.c_str(), modelFilesPath.c_str())) {
        return false;
    }

    Logger::debug(path + " vertices: "   + std::to_string(attributes.vertices.size())
                       + ", normals: "   + std::to_string(attributes.normals.size())
                       + ", texcoords: " + std::to_string(attributes.texcoords.size())
                       + ", shapes: "    + std::to_string(shapes.size()));

    bool hasNormals = !attributes.normals.empty();

    // For each shape that forms the mesh
    for (const auto& [name, objMesh] : shapes) {
        // For each face that forms the shape
        for (size_t face = 0; face < objMesh.num_face_vertices.size(); face++) {

            // For each vertex that forms the face
            for (size_t vert = 0; vert < 3; vert++) {
                auto [vertex_index, normal_index, texcoord_index] = objMesh.indices[3 * face + vert];

                Vertex vertex{};

                // Define position attribute
                vertex.position = {
                    attributes.vertices[3 * vertex_index + 0],
                    attributes.vertices[3 * vertex_index + 1],
                    attributes.vertices[3 * vertex_index + 2]
                };

                // Define normal attribute
                if (hasNormals && normal_index >= 0) {
                    vertex.normal = {
                        attributes.normals[3 * normal_index + 0],
                        attributes.normals[3 * normal_index + 1],
                        attributes.normals[3 * normal_index + 2]
                    };
                }

                // Define texture coordinates attribute
                if (!attributes.texcoords.empty() && texcoord_index >= 0) {
                    // Y coordinate of the texture must be flipped to match coordinate space
                    vertex.textureCoords = {
                               attributes.texcoords[2 * texcoord_index + 0],
                        1.0f - attributes.texcoords[2 * texcoord_index + 1]
                    };
                }

                // Define color attribute
                vertex.color = {1.0f, 1.0f, 1.0f};

                if (!uniqueVertices.contains(vertex)) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(mesh.getVertices().size());
                    mesh.addVertex(vertex);
                }

                mesh.addIndex(uniqueVertices[vertex]);
            }

            const unsigned int materialIndex = objMesh.material_ids[face];

            // Load material if ID is valid
            if (materialIndex < materials.size()) {
                const tinyobj::material_t& material = materials[materialIndex];

                loadMaterial_OBJ(mesh, material);
            }
        }
    }

    if (!hasNormals) {
        mesh.generateSmoothNormals();
    }

    model.addMesh(mesh);

    return true;
}

/*---------------------------------------*/
/*           .GLTF file loader           */
/*---------------------------------------*/

struct AttributeData {
    const unsigned char* base;
    size_t stride = 0;
};

static AttributeData getAttributeData(
    const tinygltf::Accessor& accessor,
    const tinygltf::BufferView& bufferView,
    const tinygltf::Buffer& buffer
) {
    AttributeData data{};
    data.base   = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
    data.stride = bufferView.byteStride;

    if (data.stride == 0) {
        data.stride = tinygltf::GetNumComponentsInType(accessor.type)
                    * tinygltf::GetComponentSizeInBytes(accessor.componentType);
    }

    return data;
}

void ModelManager::loadMaterial_glTF(
    Mesh&                                 mesh,
    const std::string&                    modelName,
    const tinygltf::Material&             material,
    const std::vector<tinygltf::Texture>& textures,
    const std::vector<tinygltf::Image>&   images
) {
    Material meshMaterial{};

    meshMaterial.name = material.name;

    meshMaterial.diffuse  = glm::make_vec3(material.pbrMetallicRoughness.baseColorFactor.data());
    meshMaterial.emission = glm::make_vec3(material.emissiveFactor.data());

    if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
        const tinygltf::Texture& texture = textures[material.pbrMetallicRoughness.baseColorTexture.index];
        const tinygltf::Image&   image   = images[texture.source];

        meshMaterial.albedoPath = TextureHelper::sanitizeTexturePath(image.uri, modelName);
    }

    if (material.normalTexture.index >= 0) {
        const tinygltf::Texture& texture = textures[material.normalTexture.index];
        const tinygltf::Image&   image   = images[texture.source];

        meshMaterial.normalPath = TextureHelper::sanitizeTexturePath(image.uri, modelName);
    }

    if (material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {
        const tinygltf::Texture& texture = textures[material.pbrMetallicRoughness.metallicRoughnessTexture.index];
        const tinygltf::Image&   image   = images[texture.source];

        meshMaterial.specularPath = TextureHelper::sanitizeTexturePath(image.uri, modelName);
    }

    meshMaterial.metallic  = material.pbrMetallicRoughness.metallicFactor;
    meshMaterial.roughness = material.pbrMetallicRoughness.roughnessFactor;

    mesh.setMaterial(meshMaterial);
}

bool ModelManager::load_glTF(Model& model, const std::string& path, std::string& errorMessage) {
    tinygltf::Model    glTFModel;
    tinygltf::TinyGLTF glTFloader;

    std::string warningMessage;

    const std::string fullPath = modelFilesPath + path;

    const bool modelLoaded = glTFloader.LoadASCIIFromFile(&glTFModel, &errorMessage, &warningMessage, fullPath);

    if (!warningMessage.empty()) {
        //Logger::warning(warningMessage);
    }

    if (!modelLoaded) return false;

    // For each mesh that forms the model
    for (const auto& glTFMesh : glTFModel.meshes) {

        // For each primitive that forms the mesh
        for (const auto& primitive : glTFMesh.primitives) {
            Mesh mesh{};

            std::map<std::string, int> attributes = primitive.attributes;

            // Fetch indices
            const tinygltf::Accessor&   indexAccessor   = glTFModel.accessors[primitive.indices];
            const tinygltf::BufferView& indexBufferView = glTFModel.bufferViews[indexAccessor.bufferView];
            const tinygltf::Buffer&     indexBuffer     = glTFModel.buffers[indexBufferView.buffer];

            const auto indexData = getAttributeData(indexAccessor, indexBufferView, indexBuffer);

            // Fetch vertex positions
            const tinygltf::Accessor&   positionAccessor   = glTFModel.accessors[attributes.at("POSITION")];
            const tinygltf::BufferView& positionBufferView = glTFModel.bufferViews[positionAccessor.bufferView];
            const tinygltf::Buffer&     positionBuffer     = glTFModel.buffers[positionBufferView.buffer];

            const auto positionData = getAttributeData(positionAccessor, positionBufferView, positionBuffer);

            // Fetch vertex normals
            const bool hasNormals = attributes.contains("NORMAL");
            AttributeData normalData{};

            if (hasNormals) {
                const tinygltf::Accessor&   normalAccessor   = glTFModel.accessors[attributes.at("NORMAL")];
                const tinygltf::BufferView& normalBufferView = glTFModel.bufferViews[normalAccessor.bufferView];
                const tinygltf::Buffer&     normalBuffer     = glTFModel.buffers[normalBufferView.buffer];

                normalData = getAttributeData(normalAccessor, normalBufferView, normalBuffer);
            }

            // Fetch texture coordinates if they exist
            const bool hasTextureCoords = attributes.contains("TEXCOORD_0");
            AttributeData texCoordsData{};

            if (hasTextureCoords) {
                const tinygltf::Accessor&   texCoordsAccessor   = glTFModel.accessors[attributes.at("TEXCOORD_0")];
                const tinygltf::BufferView& texCoordsBufferView = glTFModel.bufferViews[texCoordsAccessor.bufferView];
                const tinygltf::Buffer&     texCoordsBuffer     = glTFModel.buffers[texCoordsBufferView.buffer];

                texCoordsData = getAttributeData(texCoordsAccessor, texCoordsBufferView, texCoordsBuffer);
            }

            // Process vertices
            for (size_t i = 0; i < indexAccessor.count; i++) {
                uint32_t vertexIndex = 0;

                switch (indexAccessor.componentType) {
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                        vertexIndex = reinterpret_cast<const uint16_t*>(indexData.base)[i];
                        break;
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                        vertexIndex = reinterpret_cast<const uint32_t*>(indexData.base)[i];
                        break;
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                        vertexIndex = indexData.base[i];
                        break;
                    default: break;
                }

                Vertex vertex{};

                // Define position attribute
                const auto* positionPtr = reinterpret_cast<const float*>(
                    positionData.base + vertexIndex * positionData.stride
                );

                vertex.position = {positionPtr[0], positionPtr[1], positionPtr[2]};

                // Define normal attribute
                if (hasNormals) {
                    const auto* normalPtr = reinterpret_cast<const float*>(
                        normalData.base + vertexIndex * normalData.stride
                    );

                    vertex.normal = {normalPtr[0], normalPtr[1], normalPtr[2]};
                }

                // Define texture coordinates attribute
                if (hasTextureCoords) {
                    const auto* textureCoordsPtr = reinterpret_cast<const float*>(
                        texCoordsData.base + vertexIndex * texCoordsData.stride
                    );

                    vertex.textureCoords = {textureCoordsPtr[0], textureCoordsPtr[1]};
                }

                mesh.addVertex(vertex);
                mesh.addIndex(mesh.getVertices().size() - 1);
            }

            if (!hasNormals) {
                mesh.generateSmoothNormals();
            }

            const unsigned int materialIndex = primitive.material;

            // Load material if ID is valid
            if (materialIndex < glTFModel.materials.size()) {
                tinygltf::Material material = glTFModel.materials[materialIndex];

                loadMaterial_glTF(mesh, model.name, material, glTFModel.textures, glTFModel.images);
            }

            model.addMesh(mesh);
        }
    }

    return true;
}

const Model* ModelManager::load(const std::string& path, std::string& errorMessage) {
    {
        // If model is already cached, return it
        std::lock_guard lock(_mutex);

        if (_cache.contains(path)) {
            return _cache.at(path).get();
        }
    }

    // Loading model
    Model model{};

    model.retrieveName(path);

    if (path.ends_with(".obj")) {

        if (!load_OBJ(model, path, errorMessage)) {
            return nullptr;
        }

    } else if (path.ends_with(".gltf")) {

        if (!load_glTF(model, path, errorMessage)) {
            return nullptr;
        }

    } else {
        errorMessage = "Failed to load model \"" + modelFilesPath + path + "\": unsupported format";
        return nullptr;
    }

    // Inserting model data into cache
    std::lock_guard lock(_mutex);

    auto [it, inserted] = _cache.try_emplace(path, std::make_unique<Model>(std::move(model)));

    return it->second.get();
}
