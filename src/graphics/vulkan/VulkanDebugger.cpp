#include "VulkanDebugger.h"

#include <string>

namespace VulkanLogger {
    std::string formatVulkanErrorMessage(const std::string& expression, vk::Result result) {
        const int32_t errorCode = static_cast<int32_t>(result);
        return expression + " failed: " + vk::to_string(result) + " (error code: " + std::to_string(errorCode) + ")";
    }
}
