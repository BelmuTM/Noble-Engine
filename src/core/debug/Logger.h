#pragma once

#include "ErrorHandling.h"

#include <cstdint>
#include <string>

namespace Logger {
    enum class Level : std::uint8_t { DEBUG, VERBOSE, INFO, WARNING, ERROR, FATAL, COUNT };

    void init();
    void shutdown();

    void log(Level level, const std::string& message);

    void debug  (const std::string& message);
    void verbose(const std::string& message);
    void info   (const std::string& message);
    void warning(const std::string& message);

    void error(const std::string& message);
    void error(const Failure& failure);

    void fatal(const std::string& message);
    void fatal(const Failure& failure);

    class Manager {
    public:
        Manager();
        ~Manager();
    };
}
