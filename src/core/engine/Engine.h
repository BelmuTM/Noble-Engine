#pragma once

#include "core/debug/Logger.h"

#include <atomic>
#include <string>

namespace Engine {
    inline std::atomic running(true);

    inline int exitCode = EXIT_SUCCESS;

    inline static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    void fatalExit(const std::string& message);

    void fatalExit(const Failure& failure);

    template<typename T>
    void fatalOnFail(Expected<T>&& result) {
        if (result.failed()) {
            fatalExit(result.failure());
        }
    }

    inline void fatalOnFail(Expected<void>&& result) {
        if (result.failed()) {
            fatalExit(result.failure());
        }
    }
}
