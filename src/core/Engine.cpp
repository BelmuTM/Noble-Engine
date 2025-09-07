#include "Engine.h"
#include "core/debug/Logger.h"

namespace Engine {
    void localtime(std::tm& tm, const std::time_t* time) {
#if defined(_WIN32) || defined(_WIN64)
        localtime_s(&tm, time);
#else
        localtime_r(time, &tm);
#endif
    }

    void fatalExit(const std::string& message) {
        Logger::fatal(message);
        std::exit(EXIT_FAILURE);
    }
}
