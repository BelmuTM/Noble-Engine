#include "VulkanMaterialManager.h"

#include <ranges>

bool VulkanMaterialManager::create(
    const VulkanDevice& device,
    VulkanImageManager& imageManager,
    const std::uint32_t framesInFlight,
    std::string&        errorMessage
) noexcept {
    _imageManager = &imageManager;

    TRY_BOOL(_descriptorManager.create(
        device.getLogicalDevice(), getDescriptorScheme(), framesInFlight, MAX_MATERIALS, errorMessage
    ));

    return true;
}

void VulkanMaterialManager::destroy() noexcept {
    _descriptorManager.destroy();
}

VulkanMaterial* VulkanMaterialManager::getOrCreateMaterial(const Material& sourceMaterial, std::string& errorMessage) {
    const auto it = _materials.find(sourceMaterial);
    if (it != _materials.end()) return it->second.get();

    auto [emplaceIt, inserted] = _materials.emplace(sourceMaterial, std::make_unique<VulkanMaterial>());

    if (inserted) {
        if (!emplaceIt->second->create(sourceMaterial, _imageManager, _descriptorManager, errorMessage)) {
            _materials.erase(emplaceIt);
            return nullptr;
        }
    }

    return emplaceIt->second.get();
}

bool VulkanMaterialManager::loadTextures(
    const AssetManager::TexturesMap& textures, std::string& errorMessage
) const {
    std::vector<const Image*> images{};

    for (const auto& texture : textures | std::views::values) {
        if (!texture) continue;
        images.push_back(texture);
    }

    TRY_BOOL(_imageManager->loadBatchedImages(images, errorMessage));

    return true;
}
