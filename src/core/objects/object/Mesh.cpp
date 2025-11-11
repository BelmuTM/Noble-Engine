#include "Mesh.h"

#include "core/ResourceManager.h"
#include "core/debug/ErrorHandling.h"
#include "core/debug/Logger.h"

void Mesh::generateSmoothNormals(
    const size_t vertexStart,
    const size_t vertexEnd,
    const size_t indexStart,
    const size_t indexEnd
) {
    for (size_t i = indexStart; i < indexEnd; i += 3) {
        const uint32_t i0 = _indices[i + 0];
        const uint32_t i1 = _indices[i + 1];
        const uint32_t i2 = _indices[i + 2];

        if (i0 < vertexStart || i0 >= vertexEnd) continue;
        if (i1 < vertexStart || i1 >= vertexEnd) continue;
        if (i2 < vertexStart || i2 >= vertexEnd) continue;

        const glm::vec3 edge1 = _vertices[i1].position - _vertices[i0].position;
        const glm::vec3 edge2 = _vertices[i2].position - _vertices[i0].position;

        const glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

        _vertices[i0].normal += faceNormal;
        _vertices[i1].normal += faceNormal;
        _vertices[i2].normal += faceNormal;
    }

    for (uint32_t i = vertexStart; i < vertexEnd; i++) {
        _vertices[i].normal = glm::normalize(_vertices[i].normal);
    }
}

// -------------------------------
//        .OBJ file loader
// -------------------------------

void Mesh::loadMaterialObj(const tinyobj::material_t& material) {
    _material.name          = material.name;
    _material.albedoPath    = material.diffuse_texname;
    _material.normalPath    = material.normal_texname;
    _material.specularPath  = material.specular_texname;
    _material.roughnessPath = material.roughness_texname;
    _material.metallicPath  = material.metallic_texname;

    _material.ior       = material.ior;
    _material.metallic  = material.metallic;
    _material.roughness = material.roughness;
}

bool Mesh::loadObj(const std::string& path, std::string& errorMessage) {
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
    for (const auto& [name, mesh] : shapes) {
        // For each face that forms the shape
        for (size_t face = 0; face < mesh.num_face_vertices.size(); face++) {

            // For each vertex that forms the face
            for (size_t vert = 0; vert < 3; vert++) {
                auto [vertex_index, normal_index, texcoord_index] = mesh.indices[3 * face + vert];

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
                    uniqueVertices[vertex] = static_cast<uint32_t>(_vertices.size());
                    _vertices.push_back(vertex);
                }

                _indices.push_back(uniqueVertices[vertex]);
            }

            const int materialIndex = mesh.material_ids[face];

            // Load material if ID is valid
            if (materialIndex >= 0 && materialIndex < materials.size()) {
                tinyobj::material_t material = materials[materialIndex];

                loadMaterialObj(material);
            }
        }
    }

    if (!hasNormals) {
        generateSmoothNormals(0, _vertices.size(), 0, _indices.size());
    }

    return true;
}

// -------------------------------
//        .GLTF file loader
// -------------------------------

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

void Mesh::loadMaterialGLTF(
    const tinygltf::Material&             material,
    const std::vector<tinygltf::Texture>& textures,
    const std::vector<tinygltf::Image>&   images
) {
    if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
        const tinygltf::Texture& texture = textures[material.pbrMetallicRoughness.baseColorTexture.index];
        const tinygltf::Image&   image   = images[texture.source];

        _material.albedoPath = TextureHelper::sanitizeTexturePath(image.uri, _name);
    }

    if (material.normalTexture.index >= 0) {
        const tinygltf::Texture& texture = textures[material.normalTexture.index];
        const tinygltf::Image&   image   = images[texture.source];

        _material.normalPath = TextureHelper::sanitizeTexturePath(image.uri, _name);
    }
}

bool Mesh::loadGLTF(const std::string& path, std::string& errorMessage) {
    tinygltf::Model    model;
    tinygltf::TinyGLTF loader;

    std::string warningMessage;

    const std::string fullPath = modelFilesPath + path;

    const bool modelLoaded = loader.LoadASCIIFromFile(&model, &errorMessage, &warningMessage, fullPath);

    if (!warningMessage.empty()) {
        Logger::warning(warningMessage);
    }

    if (!modelLoaded) return false;

    // For each mesh that forms the model
    for (const auto& mesh : model.meshes) {
        // For each primitive that forms the mesh
        for (const auto& primitive : mesh.primitives) {
            size_t vertexStart = _vertices.size();
            size_t indexStart  = _indices.size();

            // Fetch indices
            const tinygltf::Accessor&   indexAccessor   = model.accessors[primitive.indices];
            const tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
            const tinygltf::Buffer&     indexBuffer     = model.buffers[indexBufferView.buffer];

            const auto indexData = getAttributeData(indexAccessor, indexBufferView, indexBuffer);

            // Fetch vertex positions
            const tinygltf::Accessor&   positionAccessor   = model.accessors[primitive.attributes.at("POSITION")];
            const tinygltf::BufferView& positionBufferView = model.bufferViews[positionAccessor.bufferView];
            const tinygltf::Buffer&     positionBuffer     = model.buffers[positionBufferView.buffer];

            const auto positionData = getAttributeData(positionAccessor, positionBufferView, positionBuffer);

            // Fetch vertex normals
            const bool hasNormals = primitive.attributes.contains("NORMAL");
            AttributeData normalData{};

            if (hasNormals) {
                const tinygltf::Accessor&   normalAccessor   = model.accessors[primitive.attributes.at("NORMAL")];
                const tinygltf::BufferView& normalBufferView = model.bufferViews[normalAccessor.bufferView];
                const tinygltf::Buffer&     normalBuffer     = model.buffers[normalBufferView.buffer];

                normalData = getAttributeData(normalAccessor, normalBufferView, normalBuffer);
            }

            // Fetch texture coordinates if they exist
            const bool hasTextureCoords = primitive.attributes.contains("TEXCOORD_0");
            AttributeData texCoordsData{};

            if (hasTextureCoords) {
                const tinygltf::Accessor&   texCoordsAccessor   = model.accessors[primitive.attributes.at("TEXCOORD_0")];
                const tinygltf::BufferView& texCoordsBufferView = model.bufferViews[texCoordsAccessor.bufferView];
                const tinygltf::Buffer&     texCoordsBuffer     = model.buffers[texCoordsBufferView.buffer];

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

                    // Y coordinate of the texture must be flipped to match coordinate space
                    vertex.textureCoords = {textureCoordsPtr[0], 1.0f - textureCoordsPtr[1]};
                }

                _vertices.push_back(vertex);
                _indices.push_back(_vertices.size() - 1);
            }

            size_t vertexEnd = _vertices.size();
            size_t indexEnd  = _indices.size();

            if (!hasNormals) {
                generateSmoothNormals(vertexStart, vertexEnd, indexStart, indexEnd);
            }

            const int materialIndex = primitive.material;

            // Load material if ID is valid
            if (materialIndex >= 0 && materialIndex < model.materials.size()) {
                tinygltf::Material material = model.materials[materialIndex];

                loadMaterialGLTF(material, model.textures, model.images);
            }
        }
    }

    return true;
}

bool Mesh::load(const std::string& path, std::string& errorMessage) {
    _name = std::filesystem::path(path).stem().string();

    if (path.ends_with(".obj")) {

        TRY(loadObj(path, errorMessage));

    } else if (path.ends_with(".gltf")) {

        TRY(loadGLTF(path, errorMessage));

    }

    return true;
}
