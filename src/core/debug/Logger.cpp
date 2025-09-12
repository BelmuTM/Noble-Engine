// TO-DO: move signal handling to Platform.h and write PlatformLinux.cpp

#include "Logger.h"
#include "core/Engine.h"

#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#include <Windows.h>
#undef ERROR

namespace {
    constexpr size_t MAX_LOG_QUEUE_SIZE = 512;

#ifdef LOG_FILE_WRITE
    std::ofstream logFile;
#endif
    std::thread             logThread;
    std::mutex              logMutex;
    std::condition_variable logCv;

    std::atomic<bool> running{false};

    struct Log {
        Logger::Level                         level = Logger::Level::DEBUG;
        std::string                           message;
        std::chrono::system_clock::time_point timestamp;

        Log() = default;

        Log(const Logger::Level lvl, std::string msg)
            : level(lvl), message(std::move(msg)), timestamp(std::chrono::system_clock::now()) {
        }
    };

    std::queue<Log> logQueue;

    constexpr std::array<const char*, static_cast<size_t>(Logger::Level::COUNT)> levelStrings = {
        "DEBUG", "VERROBE", "INFO", "WARNING", "ERROR", "FATAL"
    };

    template<typename Stream>
    void writeLogMessage(Stream& os, const Log& log) {
        const auto time = std::chrono::system_clock::to_time_t(log.timestamp);
        std::tm    tm{};
        Engine::localtime(tm, &time);

        std::ostringstream prefixStream;
        prefixStream << std::put_time(&tm, "[%Y-%m-%d %H:%M:%S] ")
                     << '[' << levelStrings[static_cast<size_t>(log.level)] << "]: ";

        const std::string prefix = prefixStream.str();

        // Output individual lines of the log message
        std::istringstream messageStream(log.message);
        std::string        line;
        bool               firstLine = true;

        while (std::getline(messageStream, line)) {
            // Indent multi-line messages to match the prefix's length
            os << (firstLine ? prefix : std::string(prefix.size(), ' ')) << line << '\n';
            firstLine = false;
        }
    }

    void logWorker() {
        while (running || !logQueue.empty()) {
            Log entry;
            {
                // Acquire the mutex to safely access the shared log queue
                std::unique_lock lock(logMutex);
                // Wait until there is at least one message in the queue or the logger is shutdown to log messages
                logCv.wait(lock, [] { return !logQueue.empty() || !running; });

                if (logQueue.empty()) continue;

                entry = std::move(logQueue.front());
                logQueue.pop();
            }

#ifdef LOG_FILE_WRITE
            if (logFile.is_open()) {
                writeLogMessage(logFile, entry);
            }
#endif
            writeLogMessage(std::cout, entry);

            if (entry.level >= Logger::Level::ERROR) {
#ifdef LOG_FILE_WRITE
                logFile.flush();
#endif
                std::cout.flush();
            }
        }
    }

    void setupAtexitHandler() {
        std::atexit(Logger::shutdown);
    }

    void signalHandler(int signal) {
        Logger::shutdown();
        std::_Exit(EXIT_FAILURE);
    }

    void setupSignalHandlers() {
        std::signal(SIGINT , signalHandler);
        std::signal(SIGTERM, signalHandler);
    }

    BOOL WINAPI ConsoleHandler(const DWORD ctrlType) {
        switch (ctrlType) {
            case CTRL_C_EVENT:
            case CTRL_BREAK_EVENT:
            case CTRL_CLOSE_EVENT:
            case CTRL_LOGOFF_EVENT:
            case CTRL_SHUTDOWN_EVENT:
                Logger::shutdown();
                return TRUE;
            default:
                return FALSE;
        }
    }

    void setupConsoleHandler() {
        if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
            Logger::error("Failed to set console control handler");
        }
    }
}

namespace Logger {
    void init() {
#ifdef LOG_FILE_WRITE
        const auto now  = std::chrono::system_clock::now();
        const auto time = std::chrono::system_clock::to_time_t(now);
        std::tm    tm{};
        Engine::localtime(tm, &time);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S") << ".log";
        const std::string logFileName = oss.str();

        logFile.open(logFileName);

        if (!logFile.is_open()) {
            std::cout << "[FATAL]: Failed to open log file" << std::endl;
            std::exit(EXIT_FAILURE);
        }
#endif

        running   = true;
        logThread = std::thread(logWorker);

        setupAtexitHandler();
        setupSignalHandlers();
        setupConsoleHandler();
    }

    void shutdown() {
        running = false;

        logCv.notify_all();

        if (logThread.joinable()) {
            logThread.join();
        }

#ifdef LOG_FILE_WRITE
        if (logFile.is_open()) {
            logFile.close();
        }
#endif
    }

    void log(const Level level, const std::string& message) {
        std::unique_lock lock(logMutex);

        // Drop the oldest messages if queue is overflowing
        if (logQueue.size() >= MAX_LOG_QUEUE_SIZE) {
            logQueue.pop();
        }

        logQueue.push(std::move(Log{level, message}));
        lock.unlock();
        logCv.notify_one();
    }

    std::string formatErrorMessage(const std::string& functionName, const int errorCode) {
        return functionName + " failed (error code: " + (errorCode == -1 ? "N/A" : std::to_string(errorCode)) + ")";
    }

    void debug  (const std::string& message) { log(Level::DEBUG  , message); }
    void verbose(const std::string& message) { log(Level::VERBOSE, message); }
    void info   (const std::string& message) { log(Level::INFO   , message); }
    void warning(const std::string& message) { log(Level::WARNING, message); }
    void error  (const std::string& message) { log(Level::ERROR  , message); }
    void fatal  (const std::string& message) { log(Level::FATAL  , message); }

    Manager::Manager() {
        init();
    }

    Manager::~Manager() {
        shutdown();
    }
}
