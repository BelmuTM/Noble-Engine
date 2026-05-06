#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorManager.h"
#include "graphics/vulkan/resources/materials/VulkanMaterial.h"

#include "core/resources/AssetManager.h"

class VulkanMaterialManager {
public:
    static constexpr std::uint32_t MAX_MATERIALS = 2048;

    VulkanMaterialManager()  = default;
    ~VulkanMaterialManager() = default;

    VulkanMaterialManager(const VulkanMaterialManager&)            = delete;
    VulkanMaterialManager& operator=(const VulkanMaterialManager&) = delete;

    VulkanMaterialManager(VulkanMaterialManager&&)            = delete;
    VulkanMaterialManager& operator=(VulkanMaterialManager&&) = delete;

    [[nodiscard]] Expected<void> create(
        const VulkanDevice& device,
        VulkanImageManager& imageManager,
        std::uint32_t       framesInFlight
    ) noexcept;

    void destroy() noexcept;

    Expected<VulkanMaterial*> getOrCreateMaterial(const Material& sourceMaterial);

    [[nodiscard]] Expected<void> loadTextures(const AssetManager::TexturesMap& textures) const;

    [[nodiscard]] static VulkanDescriptorScheme getDescriptorScheme() noexcept {
        VulkanDescriptorScheme scheme{};

        for (std::size_t i = 0; i < static_cast<std::size_t>(TextureType::Count); i++) {
            scheme.emplace_back(i, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
        }

        return scheme;
    }

    [[nodiscard]] const VulkanDescriptorManager& getDescriptorManager() const noexcept { return _descriptorManager; }

private:
    VulkanImageManager* _imageManager = nullptr;

    VulkanDescriptorManager _descriptorManager{};

    std::unordered_map<Material, std::unique_ptr<VulkanMaterial>, MaterialHash> _materials;
};
