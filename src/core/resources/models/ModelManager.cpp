#include "ModelManager.h"

#include "common/Utility.h"

#include "core/debug/Logger.h"
#include "core/resources/AssetPaths.h"

#include <glm/gtc/type_ptr.hpp>

std::shared_future<std::unique_ptr<Model>> ModelManager::load(const std::string& path, std::string& errorMessage) {
    return loadAsyncFuture(path, [path, &errorMessage]() -> std::unique_ptr<Model> {

        Logger::info("Loading model \"" + path + "\"...");

        // Load model
        auto model = std::make_unique<Model>();

        model->retrieveName(path);

        const std::string& extension = Utility::getFileExtension(path);
        const std::string  fullPath  = AssetPaths::MODELS + path;

        bool loaded = false;

        if (extension == ".obj") {
            loaded = load_OBJ(*model, fullPath, errorMessage);

        } else if (extension == ".gltf" || extension == ".glb") {
            loaded = load_glTF(*model, fullPath, extension, errorMessage);

        } else {
            errorMessage = "Failed to load model \"" + fullPath + "\": unsupported format.";
            loaded = false;
        }

        if (!loaded) return nullptr;

        // Add each mesh's textures to the model's texture paths (albedo, normal map, metallic)
        for (const auto& mesh : model->meshes) {
            const auto& material = mesh.getMaterial();

            model->texturePaths.insert(material.albedoPath);
            model->texturePaths.insert(material.normalPath);
            model->texturePaths.insert(material.specularPath);
        }

        return model;
    });
}

const Model* ModelManager::loadBlocking(const std::string& path, std::string& errorMessage) {
    const auto& future = load(path, errorMessage);
    if (!future.valid()) return nullptr;

    return future.get().get();
}

/*---------------------------------------*/
/*           .OBJ file loader            */
/*---------------------------------------*/

// ------ Material ------

void ModelManager::loadMaterial_OBJ(Mesh& mesh, const std::string& modelName, const tinyobj::material_t& material) {
    Material meshMaterial{};

    meshMaterial.name = material.name;

    meshMaterial.diffuse  = glm::make_vec3(material.diffuse);
    meshMaterial.specular = glm::make_vec3(material.specular);
    meshMaterial.emission = glm::make_vec3(material.emission);

    meshMaterial.albedoPath    = TextureHelper::sanitizeTexturePath(material.diffuse_texname, modelName);
    meshMaterial.normalPath    = TextureHelper::sanitizeTexturePath(material.normal_texname, modelName);
    meshMaterial.specularPath  = TextureHelper::sanitizeTexturePath(material.specular_texname, modelName);
    meshMaterial.roughnessPath = TextureHelper::sanitizeTexturePath(material.roughness_texname, modelName);
    meshMaterial.metallicPath  = TextureHelper::sanitizeTexturePath(material.metallic_texname, modelName);

    meshMaterial.ior       = material.ior;
    meshMaterial.metallic  = material.metallic;
    meshMaterial.roughness = material.roughness;

    mesh.setMaterial(meshMaterial);
}

// ------ Model ------

bool ModelManager::load_OBJ(Model& model, const std::string& path, std::string& errorMessage) {
    Mesh mesh{};

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    tinyobj::attrib_t attributes;

    std::vector<tinyobj::shape_t>    shapes{};
    std::vector<tinyobj::material_t> materials{};

    if (!tinyobj::LoadObj(
        &attributes, &shapes, &materials, &errorMessage, path.c_str(), AssetPaths::MODELS
    )) {
        return false;
    }

    bool hasNormals = !attributes.normals.empty();

    Math::AABB aabb{};

    // For each shape that forms the mesh
    for (const auto& [name, objMesh] : shapes) {
        // For each face that forms the shape
        size_t indexOffset = 0;

        for (size_t face = 0; face < objMesh.num_face_vertices.size(); face++) {
            size_t verticesCount = objMesh.num_face_vertices[face];

            // For each vertex that forms the face
            for (size_t vert = 0; vert < verticesCount; vert++) {
                auto [vertex_index, normal_index, texcoord_index] = objMesh.indices[indexOffset + vert];

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

                // Progressively find the bounds of the mesh
                aabb.minBound = glm::min(aabb.minBound, vertex.position);
                aabb.maxBound = glm::max(aabb.maxBound, vertex.position);

                // Add unique vertex and corresponding index to the mesh
                if (!uniqueVertices.contains(vertex)) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(mesh.getVertices().size());
                    mesh.addVertex(vertex);
                }

                mesh.addIndex(uniqueVertices[vertex]);
            }

            indexOffset += verticesCount;

            // Load material
            const unsigned int materialIndex = objMesh.material_ids[face];

            // If material ID is valid
            if (materialIndex < materials.size()) {
                const tinyobj::material_t& material = materials[materialIndex];

                loadMaterial_OBJ(mesh, model.name, material);
            }
        }
    }

    mesh.setAABB(aabb);

    if (!hasNormals) {
        mesh.generateSmoothNormals();
        mesh.generateTangents();
    }

    model.addMesh(mesh);

    return true;
}

/*---------------------------------------*/
/*           .GLTF file loader           */
/*---------------------------------------*/

// ------ Material ------

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

// ------ Model ------

struct AttributeData {
    const tinygltf::Accessor* accessor;
    const unsigned char* base;
    size_t stride = 0;

    [[nodiscard]] const unsigned char* getData(const size_t index) const {
        return base + index * stride;
    }
};

static AttributeData getAttributeData(const tinygltf::Model& glTFModel, const int accessorIndex) {
    const tinygltf::Accessor&   accessor   = glTFModel.accessors[accessorIndex];
    const tinygltf::BufferView& bufferView = glTFModel.bufferViews[accessor.bufferView];
    const tinygltf::Buffer&     buffer     = glTFModel.buffers[bufferView.buffer];

    AttributeData data{};
    data.accessor = &accessor;
    data.base     = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
    data.stride   = bufferView.byteStride;

    if (data.stride == 0) {
        data.stride = tinygltf::GetNumComponentsInType(accessor.type)
                    * tinygltf::GetComponentSizeInBytes(accessor.componentType);
    }

    return data;
}

void ModelManager::processMeshPrimitives_glTF(
    Mesh&                      mesh,
    const tinygltf::Model&     glTFModel,
    const tinygltf::Primitive& primitive
) {
    const std::map<std::string, int> attributes = primitive.attributes;

    // Fetch indices
    const auto indexData = getAttributeData(glTFModel, primitive.indices);

    // Fetch vertex positions
    const auto positionData = getAttributeData(glTFModel, attributes.at("POSITION"));

    // Fetch vertex normals
    const bool hasNormals = attributes.contains("NORMAL");
    AttributeData normalData{};

    if (hasNormals) {
        normalData = getAttributeData(glTFModel, attributes.at("NORMAL"));
    }

    // Fetch vertex tangents
    const bool hasTangents = attributes.contains("TANGENT");
    AttributeData tangentData{};

    if (hasTangents) {
        tangentData = getAttributeData(glTFModel, attributes.at("TANGENT"));
    }

    // Fetch texture coordinates if they exist
    const bool hasTextureCoords = attributes.contains("TEXCOORD_0");
    AttributeData texCoordsData{};

    if (hasTextureCoords) {
        texCoordsData = getAttributeData(glTFModel, attributes.at("TEXCOORD_0"));
    }

    // Process vertices
    Math::AABB aabb{};

    for (size_t i = 0; i < indexData.accessor->count; i++) {
        uint32_t vertexIndex = 0;

        switch (indexData.accessor->componentType) {
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
        const auto* positionPtr = reinterpret_cast<const float*>(positionData.getData(vertexIndex));

        vertex.position = {positionPtr[0], positionPtr[1], positionPtr[2]};

        // Define normal attribute
        if (hasNormals) {
            const auto* normalPtr = reinterpret_cast<const float*>(normalData.getData(vertexIndex));

            vertex.normal = {normalPtr[0], normalPtr[1], normalPtr[2]};
        }

        // Define tangent attribute
        if (hasTangents) {
            const auto* tangentPtr = reinterpret_cast<const float*>(tangentData.getData(vertexIndex));

            vertex.tangent = {tangentPtr[0], tangentPtr[1], tangentPtr[2], tangentPtr[3]};
        }

        // Define texture coordinates attribute
        if (hasTextureCoords) {
            const auto* textureCoordsPtr = reinterpret_cast<const float*>(texCoordsData.getData(vertexIndex));

            vertex.textureCoords = {textureCoordsPtr[0], textureCoordsPtr[1]};
        }

        // Progressively find the bounds of the mesh
        aabb.minBound = glm::min(aabb.minBound, vertex.position);
        aabb.maxBound = glm::max(aabb.maxBound, vertex.position);

        // Add vertex and corresponding index to the mesh
        mesh.addVertex(vertex);
        mesh.addIndex(i);
    }

    mesh.setAABB(aabb);

    if (!hasNormals) {
        mesh.generateSmoothNormals();
    }

    if (!hasTangents && hasTextureCoords) {
        mesh.generateTangents();
    }
}

Mesh ModelManager::createMesh_glTF(
    const std::string&         modelName,
    const tinygltf::Model&     glTFModel,
    const tinygltf::Primitive& primitive
) {
    Mesh mesh{};

    processMeshPrimitives_glTF(mesh, glTFModel, primitive);

    // Load material
    const unsigned int materialIndex = primitive.material;

    // If material ID is valid
    if (materialIndex < glTFModel.materials.size()) {
        const tinygltf::Material material = glTFModel.materials[materialIndex];

        loadMaterial_glTF(mesh, modelName, material, glTFModel.textures, glTFModel.images);
    }

    return mesh;
}

void ModelManager::processNode_glTF(
    Model&                 model,
    const tinygltf::Model& glTFModel,
    const tinygltf::Node&  node,
    const glm::mat4&       parentTransform
) {
    // Compute the node's transformation matrix to position it in the scene
    glm::mat4 nodeTransform;

    if (!node.matrix.empty()) {
        nodeTransform = glm::make_mat4x4(node.matrix.data());
    } else {
        glm::vec3 translation(0.0f);
        if (!node.translation.empty()) {
            translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
        }

        glm::quat rotation(1.0, 0.0, 0.0, 0.0);
        if (!node.rotation.empty()) {
            rotation = glm::quat(
                static_cast<float>(node.rotation[3]),
                static_cast<float>(node.rotation[0]),
                static_cast<float>(node.rotation[1]),
                static_cast<float>(node.rotation[2])
            );
        }

        glm::vec3 scale(1.0f);
        if (!node.scale.empty()) {
            scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
        }

        // Combine translation, rotation and scaling transforms
        nodeTransform = glm::translate(glm::mat4(1.0f), translation)
                      * glm::mat4_cast(rotation)
                      * glm::scale(glm::mat4(1.0f), scale);
    }

    // Combine the node's transform with its parent's
    const glm::mat4 worldTransform = parentTransform * nodeTransform;

    // Process and add the node's mesh to the model
    if (node.mesh >= 0) {
        const tinygltf::Mesh& glTFMesh = glTFModel.meshes[node.mesh];

        // For each primitive that forms the mesh
        for (const auto& glTFPrimitive : glTFMesh.primitives) {
            Mesh mesh = createMesh_glTF(model.name, glTFModel, glTFPrimitive);

            // Apply the transform to the mesh's vertices
            const auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(worldTransform)));

            for (auto& vertex : mesh.getVertices()) {
                vertex.position = glm::vec3(worldTransform * glm::vec4(vertex.position, 1.0f));
                vertex.normal   = glm::normalize(normalMatrix * vertex.normal);
                vertex.tangent  = glm::vec4(glm::normalize(normalMatrix * glm::vec3(vertex.tangent)), vertex.tangent.w);
            }

            mesh.setAABB(mesh.getAABB().transform(worldTransform));

            model.addMesh(mesh);
        }
    }

    // Recursively process the node's children
    for (const int childIndex : node.children) {
        processNode_glTF(model, glTFModel, glTFModel.nodes[childIndex], worldTransform);
    }
}

bool ModelManager::load_glTF(
    Model& model, const std::string& path, const std::string& extension, std::string& errorMessage
) {
    tinygltf::Model    glTFModel;
    tinygltf::TinyGLTF glTFloader;

    std::string warningMessage;

    bool modelLoaded = false;

    if (extension == ".gltf") {
        modelLoaded = glTFloader.LoadASCIIFromFile(&glTFModel, &errorMessage, &warningMessage, path);
    } else if (extension == ".glb") {
        modelLoaded = glTFloader.LoadBinaryFromFile(&glTFModel, &errorMessage, &warningMessage, path);
    }

    if (!warningMessage.empty()) {
        Logger::warning(warningMessage);
    }

    if (!modelLoaded) return false;

    // If the model has scenes and nodes

    if (!glTFModel.scenes.empty()) {
        const int              sceneIndex = glTFModel.defaultScene >= 0 ? glTFModel.defaultScene : 0;
        const tinygltf::Scene& glTFScene  = glTFModel.scenes[sceneIndex];

        if (!glTFScene.nodes.empty()) {
            for (const int nodeIndex : glTFScene.nodes) {
                processNode_glTF(model, glTFModel, glTFModel.nodes[nodeIndex], glm::mat4(1.0f));
            }

            return true;
        }
    }

    // If the model doesn't have scenes and/or nodes

    // For each mesh that forms the model
    for (const auto& glTFMesh : glTFModel.meshes) {

        // For each primitive that forms the mesh
        for (const auto& glTFPrimitive : glTFMesh.primitives) {
            Mesh mesh = createMesh_glTF(model.name, glTFModel, glTFPrimitive);

            model.addMesh(mesh);
        }
    }

    return true;
}
