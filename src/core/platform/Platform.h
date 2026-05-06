#pragma once

#include "core/debug/ErrorHandling.h"

#include <vector>

namespace Platform {
    Expected<void> init();

    void shutdown();

    std::vector<const char*> getRequiredExtensions();
}
