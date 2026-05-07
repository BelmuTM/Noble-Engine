#include "AssetManager.h"

#include "core/debug/Logger.h"

void AssetManager::loadModelsAsync(ThreadPool& threadPool, const std::vector<std::string>& modelPaths) {
    std::vector<std::future<ModelManager::ResourceHandlePointer>> futures;
    futures.reserve(modelPaths.size());

    for (const auto& path : modelPaths) {
        if (path.empty() || _models.contains(path)) continue;

        futures.push_back(threadPool.enqueue([this, path] {
            return _modelManager.load(path);
        }));
    }

    for (auto& future : futures) {
        ModelManager::ResourceHandlePointer handle = future.get();
        if (!handle) continue;

        // Spin-wait
        while (handle->isPending()) { std::this_thread::yield(); }

        if (handle->isFailed())
            Logger::error(handle->failure.error.message);
        else
            _models.emplace(handle->resource->path, std::move(handle));
    }
}

void AssetManager::loadTexturesAsync(ThreadPool& threadPool, const std::vector<std::string>& texturePaths) {
    std::vector<std::future<ImageManager::ResourceHandlePointer>> futures;
    futures.reserve(texturePaths.size());

    for (const auto& path : texturePaths) {
        if (path.empty() || _textures.contains(path)) continue;

        futures.push_back(threadPool.enqueue([this, path] {
            return _imageManager.load(path, MIPMAPS_ENABLED);
        }));
    }

    for (auto& future : futures) {
        ImageManager::ResourceHandlePointer handle = future.get();
        if (!handle) continue;

        // Spin-wait
        while (handle->isPending()) { std::this_thread::yield(); }

        if (handle->isFailed())
            Logger::error(handle->failure.error.message);
        else
            _textures.emplace(handle->resource->path, std::move(handle));
    }
}
