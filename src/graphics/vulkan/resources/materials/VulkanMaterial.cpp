#include "VulkanMaterial.h"

Expected<void> VulkanMaterial::create(
    const Material&          sourceMaterial,
    VulkanImageManager*      imageManager,
    VulkanDescriptorManager& descriptorManager
) {
    _sourceMaterial = sourceMaterial;

    // Load all textures
    TRY(loadTextures(imageManager));

    // Allocate descriptor sets
    TRY(descriptorManager.allocate(_descriptorSets));

    return {};
}

Expected<void> VulkanMaterial::loadTexture(
    const TextureType   type,
    const std::string&  path,
    const glm::vec3&    fallbackColor,
    VulkanImageManager* imageManager
) {
    VulkanImage* texturePtr = imageManager->getImage(path);

    // Load a single pixel image with the fallback color if the texture wasn't properly loaded
    if (!texturePtr) {
        const Image fallbackColorImage = Image::createSinglePixelImage(fallbackColor);
        TRY(imageManager->loadImage(texturePtr, &fallbackColorImage));
    }

    setTexture(type, texturePtr);

    return {};
}

Expected<void> VulkanMaterial::loadTextures(VulkanImageManager* imageManager) {
    TRY(loadTexture(TextureType::Albedo, _sourceMaterial.albedoPath, _sourceMaterial.diffuse, imageManager));
    TRY(loadTexture(TextureType::Normal, _sourceMaterial.normalPath, _sourceMaterial.normal, imageManager));
    TRY(loadTexture(TextureType::Specular, _sourceMaterial.specularPath, _sourceMaterial.specular, imageManager));
    return {};
}

void VulkanMaterial::bindDescriptorSets() const {
    for (std::size_t i = 0; i < _textureMap.textures.size(); i++) {
        if (_descriptorSets && _textureMap.textures[i]) {
            _descriptorSets->updatePerFrameDescriptorSets(
                _textureMap.textures[i]->getDescriptorInfo(static_cast<std::uint32_t>(i))
            );
        }
    }
}
