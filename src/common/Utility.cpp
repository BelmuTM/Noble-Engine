#include "Utility.h"

#include <filesystem>

namespace Utility {
    void localtime(std::tm& tm, const std::time_t* time) {
#if defined(_WIN32)
        localtime_s(&tm, time);
#else
        localtime_r(time, &tm);
#endif
    }

    std::string getFileExtension(const std::string& path) {
        return std::filesystem::path(path).extension().string();
    }

    glm::vec3 instanceColor(const void* instancePtr) {
        const auto hash = reinterpret_cast<std::size_t>(instancePtr);

        const float r = static_cast<float>(hash >> 0  & 0xFF) / 255.0f;
        const float g = static_cast<float>(hash >> 8  & 0xFF) / 255.0f;
        const float b = static_cast<float>(hash >> 16 & 0xFF) / 255.0f;

        return {r, g, b};
    }
}
