#pragma once

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>

#include <glm/glm.hpp>

struct Image {
    std::string path;

    std::unique_ptr<uint8_t[]> pixels{};

    int width    = 0;
    int height   = 0;
    int channels = 0;

    size_t byteSize = 0;

    bool hasMipmaps = false;

    static uint8_t toByte(const float value) {
        return static_cast<uint8_t>(std::clamp(value, 0.0f, 1.0f) * 255.0f);
    }

    static std::vector<uint8_t> rgbColorToBytes(const glm::vec3& color) {
        std::vector<uint8_t> bytes(4);

        bytes[0] = toByte(color.r); // Red
        bytes[1] = toByte(color.g); // Green
        bytes[2] = toByte(color.b); // Blue
        bytes[3] = 0xFF;            // Alpha

        return bytes;
    }

    static Image createSinglePixelImage(const glm::vec3& color) {
        Image image{};

        image.pixels    = std::make_unique<uint8_t[]>(4);
        image.pixels[0] = toByte(color.r);
        image.pixels[1] = toByte(color.g);
        image.pixels[2] = toByte(color.b);
        image.pixels[3] = 0xFF;

        image.width      = 1;
        image.height     = 1;
        image.channels   = 4;
        image.byteSize   = 4;
        image.hasMipmaps = false;

        return image;
    }
};
