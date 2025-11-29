#pragma once
#ifndef NOBLEENGINE_PLATFORM_H
#define NOBLEENGINE_PLATFORM_H

#include <string>
#include <vector>

namespace Platform {
    bool init(std::string& errorMessage);

    void shutdown();

    std::vector<const char*> getVulkanExtensions();
};

#endif //NOBLEENGINE_PLATFORM_H
