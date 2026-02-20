#pragma once

#include <ctime>
#include <string>

#include <glm/vec3.hpp>

namespace Utility {
    void localtime(std::tm& tm, const std::time_t* time);

    std::string getFileExtension(const std::string& path);

    glm::vec3 instanceColor(const void* instancePtr);
};
