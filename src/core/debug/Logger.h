#pragma once
#ifndef BAZARENGINE_LOGGER_H
#define BAZARENGINE_LOGGER_H

#include <string>

namespace Logger {
    enum class Level : size_t { DEBUG, INFO, WARNING, ERROR, FATAL, COUNT };

    void init();
    void shutdown();

    void debug  (const std::string& message);
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

#endif //BAZARENGINE_LOGGER_H