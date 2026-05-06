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
    if (const auto it = _materials.find(sourceMaterial); it != _materials.end()) {
        return Expected(it->second.get());
    }

    auto material = std::make_unique<VulkanMaterial>();

    TRY(material->create(sourceMaterial, _imageManager, _descriptorManager));


    auto [it, inserted] = _materials.emplace(sourceMaterial, std::move(material));

    return Expected(it->second.get());
}

Expected<void> VulkanMaterialManager::loadTextures(const AssetManager::TexturesMap& textures) const {
    std::vector<const Image*> images{};

    for (const auto& texture : textures | std::views::values) {
        if (!texture) continue;
        images.push_back(texture);
    }

    TRY(_imageManager->loadBatchedImages(images));

    return {};
}
