#include "ImageManager.h"

#include "core/debug/Logger.h"
#include "core/resources/AssetPaths.h"

#include "libraries/stbUsage.h"

ImageManager::ResourceHandlePointer ImageManager::load(const std::string& path, const bool hasMipmaps) {
    return loadAsync(path, [path, hasMipmaps]() -> Expected<ResourcePointer> {

        Logger::info("Loading texture \"" + path + "\"...");

        const std::string fullPath = AssetPaths::TEXTURES + path;

        // Load image bytes
        int width, height, channels;
        stbi_uc* pixels = stbi_load(fullPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

        if (!pixels) {
            return FAIL("Failed to load texture \"" + fullPath + "\".", "ImageManager");
        }

        const std::size_t byteSize = width * height * STBI_rgb_alpha;

        std::unique_ptr<std::uint8_t[]> pixelsPtr(pixels);

        auto image = std::make_unique<Image>();

        image->path       = path;
        image->pixels     = std::move(pixelsPtr);
        image->width      = width;
        image->height     = height;
        image->channels   = STBI_rgb_alpha;
        image->byteSize   = byteSize;
        image->hasMipmaps = hasMipmaps;

        return Expected(std::move(image));
    });
}

Expected<const Image*> ImageManager::loadBlocking(const std::string& path, const bool hasMipmaps) {
    const ResourceHandlePointer handle = load(path, hasMipmaps);

    if (!handle) {
        return FAIL("Failed to initiate load for texture \"" + path + "\"", "ImageManager");
    }

    // WARNING: Spin-wait, only acceptable at startup, do not use in engine loop
    while (handle->isPending()) { std::this_thread::yield(); }

    if (handle->isFailed()) {
        return Unexpected(handle->failure);
    }

    return Expected<const Image*>(handle->resource.get());
}
