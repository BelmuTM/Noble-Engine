#include "SignalHandlers.h"

#include "core/debug/Logger.h"

#if defined(_WIN32)
#include <Windows.h>
#undef ERROR
#endif

#include <csignal>

namespace SignalHandlers {

namespace {

std::atomic<bool>* runningPtr = nullptr;

void signalHandler(int signal) {
    if (runningPtr) {
        runningPtr->store(false, std::memory_order_relaxed);
    }
}

#if defined(_WIN32)

BOOL WINAPI ConsoleHandler(const DWORD ctrlType) {
    switch (ctrlType) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            runningPtr->store(false, std::memory_order_relaxed);
            return TRUE;
        default:
            return FALSE;
    }
}

void setupConsoleHandler() {
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        Logger::error("Failed to set Windows console CTRL handler");
    }
}

#endif

}

void setupHandlers(std::atomic<bool>& runningFlag) {
    runningPtr = &runningFlag;

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

#if defined(_WIN32)
    setupConsoleHandler();
#endif
}

}
