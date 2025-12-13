#pragma once
#ifndef NOBLEENGINE_UTILITY_H
#define NOBLEENGINE_UTILITY_H

#include <cstdint>
#include <ctime>
#include <string>

namespace Utility {
    void localtime(std::tm& tm, const std::time_t* time);

    std::string getFileExtension(const std::string& path);
};

#endif // NOBLEENGINE_UTILITY_H
