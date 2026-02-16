#pragma once

#include <atomic>

namespace SignalHandlers {
    void setupHandlers(std::atomic<bool>& runningFlag);
}
