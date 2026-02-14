#include "ImageManager.h"

#include "core/debug/Logger.h"
#include "core/resources/AssetPaths.h"

#include "libraries/stbUsage.h"

const Image* ImageManager::load(const std::string& path, std::string& errorMessage, const bool hasMipmaps) {
    return loadAsync(path, [path, &errorMessage, hasMipmaps]() -> std::unique_ptr<Image> {

        Logger::info("Loading texture \"" + path + "\"...");

        const std::string fullPath = AssetPaths::TEXTURES + path;

        // Load image bytes
        int width, height, channels;
        stbi_uc* pixels = stbi_load(fullPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

        if (!pixels) {
            errorMessage = "Failed to load texture \"" + fullPath + "\".";
            return nullptr;
        }

        const size_t byteSize = width * height * STBI_rgb_alpha;

        std::unique_ptr<uint8_t[]> pixelsPtr(pixels);

        auto image = std::make_unique<Image>();

        image->path       = path;
        image->pixels     = std::move(pixelsPtr);
        image->width      = width;
        image->height     = height;
        image->channels   = STBI_rgb_alpha;
        image->byteSize   = byteSize;
        image->hasMipmaps = hasMipmaps;

        return image;
    });
}
