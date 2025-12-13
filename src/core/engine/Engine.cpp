#include "Engine.h"

#include "core/debug/Logger.h"

namespace Engine {
    void fatalExit(const std::string& message) {
        Logger::fatal(message);
        std::exit(EXIT_FAILURE);
    }
}
