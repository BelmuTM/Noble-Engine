#include "Logger.h"

#include "common/Utility.h"

#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

// #define DISPLAY_YEAR

namespace {

constexpr std::size_t MAX_LOG_QUEUE_SIZE = 256;

struct LogEvent {
    Logger::Level level = Logger::Level::DEBUG;

    std::string message;
    std::string domain;

    std::chrono::system_clock::time_point timestamp;

    std::optional<Failure> failure;
};

#ifdef LOG_FILE_WRITE
std::ofstream logFile;
#endif

std::thread             logThread;
std::queue<LogEvent>    logQueue;
std::mutex              logMutex;
std::condition_variable logCv;

std::atomic running{false};

template<typename Stream>
void writeLog(Stream& os, const LogEvent& log) {
    const auto time = std::chrono::system_clock::to_time_t(log.timestamp);
    std::tm    tm{};

    Utility::localtime(tm, &time);

#ifdef DISPLAY_YEAR
    static constexpr auto dateFormat = "(%Y-%m-%d %H:%M:%S)";
#else
    static constexpr auto dateFormat = "(%H:%M:%S)";
#endif

    const auto         timeString  = std::put_time(&tm, dateFormat);
    const std::string& levelString = Logger::levelStrings[static_cast<std::size_t>(log.level)];

    std::ostringstream prefixStream;
    prefixStream << timeString << " [";

    if (!log.domain.empty())
        prefixStream << log.domain << " / ";

    prefixStream << levelString << "]: ";

    const std::string prefix = prefixStream.str();

    // Output individual lines of the log message
    std::istringstream messageStream(log.message);
    std::string        messageLine;
    bool               firstLine = true;

    while (std::getline(messageStream, messageLine)) {
        // Indent multi-line messages to match the prefix's length
        os << (firstLine ? prefix : std::string(prefix.size(), ' ')) << messageLine << '\n';
        firstLine = false;
    }

    if (log.failure) {
        for (std::size_t i = 0; i < log.failure->count; i++) {
            auto [function, file, line] = log.failure->frames[i];
            os << "    at " << function << " (" << file << ':' << line << ")\n";
        }
    }
}

void logWorker() {
    while (running.load() || !logQueue.empty()) {
        LogEvent entry;
        {
            // Acquire the mutex to safely access the shared log queue
            std::unique_lock lock(logMutex);
            // Wait until there is at least one message in the queue or the logger is shutdown to log messages
            logCv.wait(lock, [] { return !running.load() || !logQueue.empty(); });

            if (logQueue.empty()) continue;

            entry = std::move(logQueue.front());
            logQueue.pop();
        }

#ifdef LOG_FILE_WRITE
        if (logFile.is_open()) {
            writeLog(logFile, entry);
        }
#endif
        writeLog(std::cout, entry);

        if (entry.level >= Logger::Level::ERROR) {

#ifdef LOG_FILE_WRITE
            logFile.flush();
#endif
            std::cout.flush();
        }
    }
}

}

namespace Logger {
    static const std::string LOGS_DIRECTORY_NAME = "logs";

    void init() {

#ifdef LOG_FILE_WRITE

        const auto now  = std::chrono::system_clock::now();
        const auto time = std::chrono::system_clock::to_time_t(now);
        std::tm    tm{};

        Utility::localtime(tm, &time);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");

        const std::string logFileName = LOGS_DIRECTORY_NAME + "/" + oss.str() + ".log";

        if (!std::filesystem::exists(LOGS_DIRECTORY_NAME)) {
            std::filesystem::create_directory(LOGS_DIRECTORY_NAME);
        }

        logFile.open(logFileName);

        if (!logFile.is_open()) {
            std::cout << "[FATAL]: Failed to open log file" << std::endl;
            std::exit(EXIT_FAILURE);
        }

#endif

        running.store(true);

        logThread = std::thread(logWorker);
    }

    void shutdown() {
        running.store(false);

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

    void enqueueLogEvent(const LogEvent& logEvent) {
        if (logEvent.message.empty()) return;

        std::unique_lock lock(logMutex);

        // Drop the oldest messages if queue is overflowing
        if (logQueue.size() >= MAX_LOG_QUEUE_SIZE) {
            logQueue.pop();
        }

        logQueue.push(logEvent);
        logCv.notify_one();
    }

    void log(const Level level, const std::string& message) {
        return enqueueLogEvent(LogEvent{level, message, "", std::chrono::system_clock::now()});
    }

    void log(const Level level, const Failure& failure) {
        return enqueueLogEvent(LogEvent{
            level,
            failure.error.message,
            failure.error.domain,
            std::chrono::system_clock::now(),
            failure
        });
    }

    void debug  (const std::string& message) { log(Level::DEBUG, message); }
    void verbose(const std::string& message) { log(Level::VERBOSE, message); }
    void info   (const std::string& message) { log(Level::INFO, message); }
    void warning(const std::string& message) { log(Level::WARNING, message); }

    void error(const std::string& message) { log(Level::ERROR, message); }
    void error(const Failure& failure)     { log(Level::ERROR, failure); }

    void fatal(const std::string& message) { log(Level::FATAL, message); }
    void fatal(const Failure& failure)     { log(Level::FATAL, failure); }

    Manager::Manager() {
        init();
    }

    Manager::~Manager() {
        shutdown();
    }
}
