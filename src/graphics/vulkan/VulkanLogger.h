#pragma once
#ifndef BAZARENGINE_VULKANLOGGER_H
#define BAZARENGINE_VULKANLOGGER_H

#define VK_ERROR_MESSAGE(func, result) VulkanLogger::formatVulkanErrorMessage(#func, result)

#include <string>

#include <vulkan/vulkan_core.h>

namespace VulkanLogger {
    std::string formatVulkanErrorMessage(const std::string& functionName, VkResult result);
}

#endif //BAZARENGINE_VULKANLOGGER_H
