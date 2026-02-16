#pragma once

#include <cstdint>
#include <ctime>
#include <string>

namespace Utility {
    void localtime(std::tm& tm, const std::time_t* time);

    std::string getFileExtension(const std::string& path);
};
