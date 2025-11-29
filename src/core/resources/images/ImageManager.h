#pragma once
#ifndef NOBLEENGINE_IMAGEMANAGER_H
#define NOBLEENGINE_IMAGEMANAGER_H

#include "Image.h"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

class ImageManager {
public:
    ImageManager()  = default;
    ~ImageManager() = default;

    ImageManager(const ImageManager&)            = delete;
    ImageManager& operator=(const ImageManager&) = delete;

    ImageManager(ImageManager&&)            = delete;
    ImageManager& operator=(ImageManager&&) = delete;

    [[nodiscard]] const Image* load(const std::string& path, std::string& errorMessage);

    [[nodiscard]] const Image* loadWithFallbackColor(
        const std::string& path, const glm::vec3& color, std::string& errorMessage
    );

private:
    std::unordered_map<std::string, std::unique_ptr<Image>> _cache{};

    std::mutex _mutex{};
};

#endif // NOBLEENGINE_IMAGEMANAGER_H
