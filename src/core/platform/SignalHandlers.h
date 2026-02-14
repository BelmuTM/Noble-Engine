#pragma once
#ifndef NOBLEENGINE_SIGNALHANDLERS_H
#define NOBLEENGINE_SIGNALHANDLERS_H

#include <atomic>

namespace SignalHandlers {
    void setupHandlers(std::atomic<bool>& runningFlag);
}

#endif // NOBLEENGINE_SIGNALHANDLERS_H
