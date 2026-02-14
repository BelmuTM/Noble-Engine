#include "core/Runtime.h"
#include "core/engine/Engine.h"
#include "core/debug/Logger.h"

#include "core/platform/Platform.h"
#include "core/platform/SignalHandlers.h"

#include <atomic>

std::atomic running(true);

int main() {
    SignalHandlers::setupHandlers(running);

    Logger::Manager loggerManager;

    std::string errorMessage;

    if (!Platform::init(errorMessage)) {
        Engine::fatalExit(errorMessage);
    }

    Runtime runtime(running);

    if (!runtime.init(errorMessage)) {
        Engine::fatalExit(errorMessage);
    }

    runtime.run();

    runtime.shutdown();

    Platform::shutdown();

    return 0;
}
