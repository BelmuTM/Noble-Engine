#include "Utility.h"

#include <filesystem>

namespace Utility {
    void localtime(std::tm& tm, const std::time_t* time) {
#if defined(_WIN32) || defined(_WIN64)
        localtime_s(&tm, time);
#else
        localtime_r(time, &tm);
#endif
    }

    std::string getFileExtension(const std::string& path) {
        return std::filesystem::path(path).extension();
    }
}
