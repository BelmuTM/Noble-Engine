#include "VulkanMaterial.h"

bool VulkanMaterial::create(
    const Material&          sourceMaterial,
    VulkanImageManager*      imageManager,
    VulkanDescriptorManager& descriptorManager,
    std::string&             errorMessage
) {
    _sourceMaterial = sourceMaterial;

    // Load all textures
    TRY_BOOL(loadTextures(imageManager, errorMessage));

    // Allocate descriptor sets
    TRY_BOOL(descriptorManager.allocate(_descriptorSets, errorMessage));

    return true;
}


bool VulkanMaterial::loadTexture(
    const TextureType   type,
    const std::string&  path,
    const glm::vec3&    fallbackColor,
    VulkanImageManager* imageManager,
    std::string&        errorMessage
) {
    VulkanImage* texturePtr = imageManager->getImage(path);

    // Load a single pixel image with the fallback color if the texture wasn't properly loaded
    if (!texturePtr) {
        const Image fallbackColorImage = Image::createSinglePixelImage(fallbackColor);
        TRY_BOOL(imageManager->loadImage(texturePtr, &fallbackColorImage, errorMessage));
    }

    setTexture(type, texturePtr);

    return true;
}

bool VulkanMaterial::loadTextures(VulkanImageManager* imageManager, std::string& errorMessage) {

    TRY_BOOL(loadTexture(
        TextureType::Albedo, _sourceMaterial.albedoPath, _sourceMaterial.diffuse, imageManager, errorMessage
    ));
    TRY_BOOL(loadTexture(
        TextureType::Normal, _sourceMaterial.normalPath, glm::vec3(0.0f), imageManager, errorMessage
    ));
    TRY_BOOL(loadTexture(
        TextureType::Specular, _sourceMaterial.specularPath, _sourceMaterial.specular, imageManager, errorMessage
    ));

    return true;
}

void VulkanMaterial::bindDescriptorSets() const {
    for (size_t i = 0; i < _textureMaps.textures.size(); i++) {
        if (_descriptorSets && _textureMaps.textures[i]) {
            _descriptorSets->updatePerFrameDescriptorSets(_textureMaps.textures[i]->getDescriptorInfo(i));
        }
    }
}
