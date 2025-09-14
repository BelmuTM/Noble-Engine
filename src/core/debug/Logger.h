#pragma once
#ifndef NOBLEENGINE_LOGGER_H
#define NOBLEENGINE_LOGGER_H

#define ERROR_MESSAGE(func) Logger::formatErrorMessage(#func)
#define ERROR_MESSAGE_CODE(func, errorCode) Logger::formatErrorMessage(#func, errorCode)

#include <string>

namespace Logger {
    enum class Level : size_t { DEBUG, VERBOSE, INFO, WARNING, ERROR, FATAL, COUNT };

    void init();
    void shutdown();

    std::string formatErrorMessage(const std::string& functionName, int errorCode = -1);

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

#endif //NOBLEENGINE_LOGGER_H