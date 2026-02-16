#pragma once

#include "Image.h"
#include "core/resources/AsyncResourceManager.h"

#include <string>

class ImageManager : public AsyncResourceManager<Image> {
public:
    ImageManager()  = default;
    ~ImageManager() = default;

    ImageManager(const ImageManager&)            = delete;
    ImageManager& operator=(const ImageManager&) = delete;

    ImageManager(ImageManager&&)            = delete;
    ImageManager& operator=(ImageManager&&) = delete;

    std::shared_future<std::unique_ptr<Image>> load(
        const std::string& path, std::string& errorMessage, bool hasMipmaps = false
    );

    const Image* loadBlocking(const std::string& path, std::string& errorMessage, bool hasMipmaps = false);
};
