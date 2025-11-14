#pragma once
#ifndef NOBLEENGINE_IMAGE_H
#define NOBLEENGINE_IMAGE_H

#include <algorithm>
#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

struct Image {
    std::vector<uint8_t> pixels{};
    int width    = 0;
    int height   = 0;
    int channels = 0;

    static std::vector<uint8_t> rgbColorToBytes(const glm::vec3& color) {
        auto toByte = [](const float value) -> uint8_t {
            return static_cast<uint8_t>(std::clamp(value, 0.0f, 1.0f) * 255.0f);
        };

        std::vector<uint8_t> bytes(4);
        bytes[0] = toByte(color.r); // Red
        bytes[1] = toByte(color.g); // Green
        bytes[2] = toByte(color.b); // Blue
        bytes[3] = 0xFF;            // Alpha

        return bytes;
    }

    static Image createSinglePixelImage(const glm::vec3& color) {
        Image image{};
        image.pixels   = rgbColorToBytes(color);
        image.width    = 1;
        image.height   = 1;
        image.channels = 4;
        return image;
    }
};

#endif // NOBLEENGINE_IMAGE_H
