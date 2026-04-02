#pragma once

#include "common/HashUtils.h"

#include "core/resources/AssetPaths.h"

#include <filesystem>
#include <string>

#include <glm/glm.hpp>

enum class TextureType {
    Albedo,
    Normal,
    Specular,
    Count
};

struct Material {
    std::string name = "Undefined_Material";

    glm::vec3 diffuse{1.0f};
    glm::vec3 specular{0.0f};
    glm::vec3 emission{0.0f};

    std::string albedoPath;
    std::string normalPath;
    std::string specularPath;
    std::string roughnessPath;
    std::string metallicPath;

    double ior       = 1.0;
    double metallic  = 0.0;
    double roughness = 0.0;

    bool operator==(const Material& other) const noexcept = default;
};

struct MaterialHash {
    std::size_t operator()(const Material& m) const noexcept {
        std::size_t hash = 0;

        HashUtils::combine(hash, m.name);
        HashUtils::combine(hash, m.albedoPath);
        HashUtils::combine(hash, m.normalPath);
        HashUtils::combine(hash, m.specularPath);
        HashUtils::combine(hash, m.roughnessPath);
        HashUtils::combine(hash, m.metallicPath);


        HashUtils::combine(hash, m.diffuse);
        HashUtils::combine(hash, m.specular);
        HashUtils::combine(hash, m.emission);

        HashUtils::combine(hash, m.ior);
        HashUtils::combine(hash, m.metallic);
        HashUtils::combine(hash, m.roughness);

        return hash;
    }
};

namespace TextureHelper {
    inline auto sanitizeTexturePath = [](const std::string& texturePath, const std::string& modelName) -> std::string {
        if (texturePath.empty()) return "";

        // Fetch texture in textures/texturePath
        std::filesystem::path initialPath = AssetPaths::TEXTURES;
        initialPath /= texturePath;
        // Found texture
        if (std::filesystem::exists(initialPath)) {
            return texturePath;
        }

        // Fetch texture in textures/modelName/texturePath
        const std::filesystem::path textureName = std::filesystem::path(texturePath).filename();

        std::filesystem::path candidatePath = AssetPaths::TEXTURES;
        candidatePath /= modelName;
        candidatePath /= textureName;
        // Found texture
        if (std::filesystem::exists(candidatePath)) {
            return modelName + '/' + textureName.string();
        }

        return "";
    };
}
