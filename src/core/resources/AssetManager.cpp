#include "AssetManager.h"

#include "core/debug/Logger.h"

void AssetManager::loadModelsAsync(ThreadPool& threadPool, const std::vector<std::string>& modelPaths) {
    std::unordered_map<std::string, std::shared_future<const Model*>> modelFutures;

    for (const auto& modelPath : modelPaths) {
        if (modelPath.empty()) continue;

        if (!modelFutures.contains(modelPath)) {
            modelFutures[modelPath] = threadPool.enqueue([this, modelPath] {
                Expected<const Model*> model = _modelManager.loadBlocking(modelPath);

                if (model.failed()) {
                    Logger::error(model.failure());
                }

                return model.value();
            }).share();
        }
    }

    for (auto& [modelPath, modelFuture] : modelFutures) {
        if (modelFuture.valid()) {
            _models.emplace(modelPath, modelFuture.get());
        }
    }
}

void AssetManager::loadTexturesAsync(ThreadPool& threadPool, const std::vector<std::string>& texturePaths) {
    std::unordered_map<std::string, std::shared_future<const Image*>> textureFutures;

    for (const auto& texturePath : texturePaths) {
        if (texturePath.empty()) continue;

        if (!textureFutures.contains(texturePath)) {
            textureFutures[texturePath] = threadPool.enqueue([this, texturePath] {
                Expected<const Image*> texture = _imageManager.loadBlocking(texturePath, MIPMAPS_ENABLED);

                if (texture.failed()) {
                    Logger::error(texture.failure());
                }

                return texture.value();
            }).share();
        }
    }

    for (auto& [texturePath, textureFuture] : textureFutures) {
        if (textureFuture.valid()) {
            _textures.emplace(texturePath, textureFuture.get());
        }
    }
}
