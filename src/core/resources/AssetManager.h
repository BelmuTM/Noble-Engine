#pragma once
#ifndef NOBLEENGINE_ASSETMANAGER_H
#define NOBLEENGINE_ASSETMANAGER_H

#include "core/resources/images/ImageManager.h"
#include "core/resources/models/ModelManager.h"

#include <string>

class AssetManager {
public:
    using ModelsMap   = std::unordered_map<std::string, const Model*>;
    using TexturesMap = std::unordered_map<std::string, const Image*>;

    AssetManager()  = default;
    ~AssetManager() = default;

    AssetManager(const AssetManager&)            = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    AssetManager(AssetManager&&)            = delete;
    AssetManager& operator=(AssetManager&&) = delete;

    [[nodiscard]] ModelManager& getModelManager() noexcept { return _modelManager; }
    [[nodiscard]] ImageManager& getImageManager() noexcept { return _imageManager; }

    [[nodiscard]]       ModelsMap& getModels()       noexcept { return _models; }
    [[nodiscard]] const ModelsMap& getModels() const noexcept { return _models; }

    [[nodiscard]]       TexturesMap& getTextures()       noexcept { return _textures; }
    [[nodiscard]] const TexturesMap& getTextures() const noexcept { return _textures; }

private:
    ModelManager _modelManager{};
    ImageManager _imageManager{};

    ModelsMap   _models{};
    TexturesMap _textures{};
};

#endif //NOBLEENGINE_ASSETMANAGER_H
