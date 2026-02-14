#include "Platform.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Platform {
    bool init(std::string& errorMessage) {
        if (!glfwInit()) {
            errorMessage = "Failed to init GLFW context.";
            return false;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        return true;
    }

    void shutdown() {
        glfwTerminate();
    }

    std::vector<const char*> getVulkanExtensions() {
        uint32_t count;
        const char** extensions = glfwGetRequiredInstanceExtensions(&count);
        if (!extensions || count == 0) return {};
        return {extensions, extensions + count};
    }
}
