#include "ImageManager.h"

#include "core/common/libraries/stbUsage.h"

#include "core/debug/Logger.h"
#include "core/engine/ResourceManager.h"

const Image* ImageManager::load(const std::string& path, std::string& errorMessage) {
    if (path.empty()) return nullptr;

    {
        // If image is already cached, return it
        std::lock_guard lock(_mutex);

        if (_cache.contains(path)) {
            return _cache.at(path).get();
        }
    }

    Logger::info("Loading texture \"" + path + "\"...");

    const std::string fullPath = textureFilesPath + path;

    // Reading file bytes
    int width, height, channels;
    stbi_uc* pixels = stbi_load(fullPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!pixels) {
        errorMessage = "Failed to load texture \"" + fullPath + "\"";
        return nullptr;
    }

    std::vector pixelsVector(pixels, pixels + width * height * STBI_rgb_alpha);

    stbi_image_free(pixels);

    auto image = std::make_unique<Image>();

    image->pixels   = std::move(pixelsVector);
    image->width    = width;
    image->height   = height;
    image->channels = STBI_rgb_alpha;

    // Inserting image data into cache
    std::lock_guard lock(_mutex);

    auto [it, inserted] = _cache.try_emplace(path, std::move(image));

    return it->second.get();
}
