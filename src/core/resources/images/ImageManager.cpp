#include "ImageManager.h"

#include "core/common/libraries/stbUsage.h"
#include "core/resources/ResourceManager.h"

#include "core/debug/Logger.h"

const Image* ImageManager::load(const std::string& path, std::string& errorMessage) {
    return loadAsync(path, [path, &errorMessage]() -> std::unique_ptr<Image> {

        Logger::info("Loading texture \"" + path + "\"...");

        const std::string fullPath = textureFilesPath + path;

        // Load image bytes
        int width, height, channels;
        stbi_uc* pixels = stbi_load(fullPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

        if (!pixels) {
            errorMessage = "Failed to load texture \"" + fullPath + "\"";
            return nullptr;
        }

        const size_t byteSize = width * height * STBI_rgb_alpha;

        std::vector pixelsVector(pixels, pixels + byteSize);

        stbi_image_free(pixels);

        auto image = std::make_unique<Image>();

        image->path     = path;
        image->pixels   = std::move(pixelsVector);
        image->width    = width;
        image->height   = height;
        image->channels = STBI_rgb_alpha;
        image->byteSize = byteSize;

        return image;
    });
}
