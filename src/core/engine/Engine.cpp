#include "Engine.h"

namespace Engine {
    void requestExit() {
        running.store(false, std::memory_order_relaxed);
    }

    void fatalExit(const std::string& message) {
        exitCode = EXIT_FAILURE;
        requestExit();

        Logger::fatal(message);
    }

    void fatalExit(const Failure& failure) {
        exitCode = EXIT_FAILURE;
        requestExit();

        Logger::fatal(failure);
    }
}
