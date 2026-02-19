#pragma once

#define ERROR_MESSAGE(func) Logger::formatErrorMessage(#func)
#define ERROR_MESSAGE_CODE(func, errorCode) Logger::formatErrorMessage(#func, errorCode)

#include <cstdint>
#include <string>

namespace Logger {
    enum class Level : uint8_t { DEBUG, VERBOSE, INFO, WARNING, ERROR, FATAL, COUNT };

    void init();
    void shutdown();

    std::string formatErrorMessage(const std::string& functionName, int errorCode = -1);

    void log(Level level, const std::string& message);

    void debug  (const std::string& message);
    void verbose(const std::string& message);
    void info   (const std::string& message);
    void warning(const std::string& message);
    void error  (const std::string& message);
    void fatal  (const std::string& message);

    class Manager {
    public:
        Manager();
        ~Manager();
    };
}
