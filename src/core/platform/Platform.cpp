#include "Platform.h"

#include <cstdint>

#include <GLFW/glfw3.h>

namespace Platform {
    Expected<void> init() {
        if (!glfwInit()) {
            return FAIL("Failed to init GLFW context.", "GLFW");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        return {};
    }

    void shutdown() {
        glfwTerminate();
    }

    std::vector<const char*> getRequiredExtensions() {
        std::uint32_t count;
        const char** extensions = glfwGetRequiredInstanceExtensions(&count);
        if (!extensions || count == 0) return {};
        return {extensions, extensions + count};
    }
}
