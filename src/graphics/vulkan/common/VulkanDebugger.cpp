#include "VulkanDebugger.h"

#include <string>

namespace VulkanDebugger {
    std::string formatVulkanErrorMessage(const std::string& expression, const vk::Result result) {
        const auto errorCode = static_cast<int32_t>(result);
        return expression + " failed: " + vk::to_string(result) + " (error code: " + std::to_string(errorCode) + ").";
    }
}
