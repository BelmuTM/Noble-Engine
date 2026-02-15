#include "AssetManager.h"

#include "core/debug/Logger.h"

void AssetManager::loadModelsAsync(ThreadPool& threadPool, const std::vector<std::string>& modelPaths) {
    std::unordered_map<std::string, std::shared_future<const Model*>> modelFutures;

    for (const auto& modelPath : modelPaths) {
        if (modelPath.empty()) continue;

        if (!modelFutures.contains(modelPath)) {
            modelFutures[modelPath] = threadPool.enqueue([this, modelPath] {
                std::string errorMessage;

                const Model* model = _modelManager.loadBlocking(modelPath, errorMessage);
                if (!model) Logger::error(errorMessage);

                return model;
            }).share();
        }
    }

    for (auto& [modelPath, modelFuture] : modelFutures) {
        _models.emplace(modelPath, modelFuture.get());
    }
}

void AssetManager::loadTexturesAsync(ThreadPool& threadPool, const std::vector<std::string>& texturePaths) {
    std::unordered_map<std::string, std::shared_future<const Image*>> textureFutures;

    for (const auto& texturePath : texturePaths) {
        if (texturePath.empty()) continue;

        if (!textureFutures.contains(texturePath)) {
            textureFutures[texturePath] = threadPool.enqueue([this, texturePath] {
                std::string errorMessage;

                const Image* texture = _imageManager.loadBlocking(texturePath, errorMessage, MIPMAPS_ENABLED);
                if (!texture) Logger::warning(errorMessage);

                return texture;
            }).share();
        }
    }

    for (auto& [texturePath, textureFuture] : textureFutures) {
        _textures.emplace(texturePath, textureFuture.get());
    }
}
