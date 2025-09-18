#pragma once
#ifndef NOBLEENGINE_VULKANDEBUGGER_H
#define NOBLEENGINE_VULKANDEBUGGER_H

#include <string>

#include <vulkan/vulkan.hpp>

namespace VulkanDebugger {
    std::string formatVulkanErrorMessage(const std::string& expression, vk::Result result);

    template <typename Func>
    auto checkVulkanResult(const char* exprStr, Func&& func, std::string& errorMessage) {
        auto returnValue = func();

        if constexpr (std::is_same_v<decltype(returnValue), vk::Result>) {
            if (returnValue != vk::Result::eSuccess) {
                errorMessage = formatVulkanErrorMessage(exprStr, returnValue);
            }
        } else {
            // assume vk::ResultValue<T>
            if (returnValue.result != vk::Result::eSuccess) {
                errorMessage = formatVulkanErrorMessage(exprStr, returnValue.result);
            }
        }
        return returnValue;
    }
}

#define VK_ERROR_MESSAGE(expr, result) VulkanDebugger::formatVulkanErrorMessage(#expr, result)

#define VK_CHECK_RESULT(expr, errorVar) VulkanDebugger::checkVulkanResult(#expr, [&]{ return expr; }, errorVar)

#endif //NOBLEENGINE_VULKANDEBUGGER_H
