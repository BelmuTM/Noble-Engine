#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorSets.h"

#include "graphics/vulkan/resources/images/VulkanImage.h"
#include "graphics/vulkan/resources/images/VulkanImageManager.h"

#include "core/resources/models/Material.h"

struct VulkanMaterialTextures {
    std::array<VulkanImage*, static_cast<std::size_t>(TextureType::Count)> textures{};
};

class VulkanMaterial {
public:
    VulkanMaterial()  = default;
    ~VulkanMaterial() = default;

    Expected<void> create(
        const Material&          sourceMaterial,
        VulkanImageManager*      imageManager,
        VulkanDescriptorManager& descriptorManager
    );

    void bindDescriptorSets() const;

    [[nodiscard]] VulkanImage* getTexture(const TextureType type) const {
        return _textureMaps.textures[static_cast<std::size_t>(type)];
    }

    void setTexture(const TextureType type, VulkanImage* image) {
        _textureMaps.textures[static_cast<std::size_t>(type)] = image;
    }

    [[nodiscard]] const VulkanDescriptorSets* getDescriptorSets() const noexcept { return _descriptorSets; }

private:
    Expected<void> loadTexture(
        TextureType         type,
        const std::string&  path,
        const glm::vec3&    fallbackColor,
        VulkanImageManager* imageManager
    );

    Expected<void> loadTextures(VulkanImageManager* imageManager);

    Material _sourceMaterial{};

    VulkanMaterialTextures _textureMaps{};

    VulkanDescriptorSets* _descriptorSets = nullptr;
};
