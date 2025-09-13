#pragma once
#ifndef BAZARENGINE_VULKANDEBUGGER_H
#define BAZARENGINE_VULKANDEBUGGER_H

#include <string>

#include <vulkan/vulkan.hpp>

namespace VulkanDebugger {
    inline std::string formatVulkanErrorMessage(const std::string& expression, vk::Result result) {
        return "Vulkan call failed: " + expression + ", result=" + vk::to_string(result);
    }

    template <typename F>
    auto checkVulkanResult(const char* exprStr, F&& func, std::string& errorMessage) {
        auto returnValue = func();
        if (returnValue.result != vk::Result::eSuccess) {
            errorMessage = formatVulkanErrorMessage(exprStr, returnValue.result);
        }
        return returnValue;
    }
}

#define VK_ERROR_MESSAGE(expr, result) VulkanDebugger::formatVulkanErrorMessage(#expr, result)

#define VK_CHECK_RESULT(expr, errorVar) VulkanDebugger::checkVulkanResult(#expr, [&]{ return expr; }, errorVar)

#endif //BAZARENGINE_VULKANDEBUGGER_H
