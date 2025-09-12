#include "VulkanLogger.h"

#include <string>
#include <unordered_map>

#include <vulkan/vulkan_core.h>

inline const char* vkResultToString(VkResult result) {
    static const std::unordered_map<VkResult, const char*> vkResultStrings = {
        {VK_SUCCESS,                         "VK_SUCCESS"},
        {VK_NOT_READY,                       "VK_NOT_READY"},
        {VK_TIMEOUT,                         "VK_TIMEOUT"},
        {VK_EVENT_SET,                       "VK_EVENT_SET"},
        {VK_EVENT_RESET,                     "VK_EVENT_RESET"},
        {VK_INCOMPLETE,                      "VK_INCOMPLETE"},
        {VK_ERROR_OUT_OF_HOST_MEMORY,        "VK_ERROR_OUT_OF_HOST_MEMORY"},
        {VK_ERROR_OUT_OF_DEVICE_MEMORY,      "VK_ERROR_OUT_OF_DEVICE_MEMORY"},
        {VK_ERROR_INITIALIZATION_FAILED,     "VK_ERROR_INITIALIZATION_FAILED"},
        {VK_ERROR_DEVICE_LOST,               "VK_ERROR_DEVICE_LOST"},
        {VK_ERROR_MEMORY_MAP_FAILED,         "VK_ERROR_MEMORY_MAP_FAILED"},
        {VK_ERROR_LAYER_NOT_PRESENT,         "VK_ERROR_LAYER_NOT_PRESENT"},
        {VK_ERROR_EXTENSION_NOT_PRESENT,     "VK_ERROR_EXTENSION_NOT_PRESENT"},
        {VK_ERROR_FEATURE_NOT_PRESENT,       "VK_ERROR_FEATURE_NOT_PRESENT"},
        {VK_ERROR_INCOMPATIBLE_DRIVER,       "VK_ERROR_INCOMPATIBLE_DRIVER"},
        {VK_ERROR_TOO_MANY_OBJECTS,          "VK_ERROR_TOO_MANY_OBJECTS"},
        {VK_ERROR_FORMAT_NOT_SUPPORTED,      "VK_ERROR_FORMAT_NOT_SUPPORTED"},
        {VK_ERROR_FRAGMENTED_POOL,           "VK_ERROR_FRAGMENTED_POOL"},
    };

    const auto it = vkResultStrings.find(result);
    if (it != vkResultStrings.end()) {
        return it->second;
    }
    return "UNKNOWN_VK_ERROR";
}

namespace VulkanLogger {
    std::string formatVulkanErrorMessage(const std::string& functionName, VkResult result) {
        return functionName + " failed: " + vkResultToString(result) + " (error code: " + std::to_string(result) + ")";
    }
}
