#include "VulkanMaterialManager.h"

#include <ranges>

Expected<void> VulkanMaterialManager::create(
    const VulkanDevice& device,
    VulkanImageManager& imageManager,
    const std::uint32_t framesInFlight
) noexcept {
    _imageManager = &imageManager;

    TRY(_descriptorManager.create(device.getLogicalDevice(), getDescriptorScheme(), framesInFlight, MAX_MATERIALS));

    return {};
}

void VulkanMaterialManager::destroy() noexcept {
    _descriptorManager.destroy();
}

Expected<VulkanMaterial*> VulkanMaterialManager::getOrCreateMaterial(const Material& sourceMaterial) {
    // If material is already cached, return it
    if (const auto cachedMaterial = _materials.find(sourceMaterial); cachedMaterial != _materials.end()) {
        return Expected(cachedMaterial->second.get());
    }

    // Otherwise, insert and create material
    auto [cachedMaterial, inserted] = _materials.emplace(sourceMaterial, std::make_unique<VulkanMaterial>());

    if (inserted) {
        TRY_CATCH(
            cachedMaterial->second->create(sourceMaterial, _imageManager, _descriptorManager),
            _materials.erase(cachedMaterial)
        );
    }

    return Expected(cachedMaterial->second.get());
}

Expected<void> VulkanMaterialManager::loadTextures(const AssetManager::TexturesMap& textures) const {
    std::vector<const Image*> images{};

    for (const auto& texture : textures | std::views::values) {
        if (!texture) continue;
        images.push_back(texture->resource.get());
    }

    TRY(_imageManager->loadBatchedImages(images));

    return {};
}
