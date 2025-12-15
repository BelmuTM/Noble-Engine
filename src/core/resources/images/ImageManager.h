#pragma once
#ifndef NOBLEENGINE_IMAGEMANAGER_H
#define NOBLEENGINE_IMAGEMANAGER_H

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

    [[nodiscard]] const Image* load(const std::string& path, std::string& errorMessage, bool hasMipmaps = false);
};

#endif // NOBLEENGINE_IMAGEMANAGER_H
