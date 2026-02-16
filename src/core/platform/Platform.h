#pragma once

#include <string>
#include <vector>

namespace Platform {
    bool init(std::string& errorMessage);

    void shutdown();

    std::vector<const char*> getRequiredExtensions();
};
