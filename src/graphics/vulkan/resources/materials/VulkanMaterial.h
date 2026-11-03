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

    [[nodiscard]] std::uint32_t getId() const noexcept { return _id; }
    void setId(const std::uint32_t id) noexcept { _id = id; }

    [[nodiscard]] const std::string& getName() const noexcept { return _sourceMaterial.name; }

    [[nodiscard]] VulkanImage* getTexture(const TextureType type) const {
        return _textureMap.textures[static_cast<std::size_t>(type)];
    }

    void setTexture(const TextureType type, VulkanImage* image) {
        _textureMap.textures[static_cast<std::size_t>(type)] = image;
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

    std::uint32_t _id = 0;

    Material _sourceMaterial{};

    VulkanMaterialTextures _textureMap{};

    VulkanDescriptorSets* _descriptorSets = nullptr;
};
