#pragma once

#include <string>

#include <glm/vec3.hpp>
#include <vulkan/vulkan.hpp>

namespace VulkanDebugger {
    std::string formatVulkanErrorMessage(const std::string& expression, vk::Result result);

    template<typename ReturnValue>
    vk::Result getVulkanResult(ReturnValue returnValue) {
        if constexpr (std::is_same_v<ReturnValue, vk::Result>) {
            // HPP style
            return returnValue;
        } else if constexpr (std::is_same_v<ReturnValue, VkResult>) {
            // C style
            return static_cast<vk::Result>(returnValue);
        } else {
            // Assume vk::ResultValue<T>
            return returnValue.result;
        }
    }

    template<typename Function>
    auto checkVulkanResult(const char* exprStr, Function&& func, std::string& errorMessage) {
        auto returnValue = func();

        const vk::Result result = getVulkanResult(returnValue);
        if (result != vk::Result::eSuccess) {
            errorMessage = formatVulkanErrorMessage(exprStr, result);
        }
        return returnValue;
    }

    template<typename ReturnValue>
    vk::Result checkVulkanResultVoid(const char* exprStr, ReturnValue returnValue, std::string& errorMessage) {
        const vk::Result result = getVulkanResult(returnValue);
        if (result != vk::Result::eSuccess) {
            errorMessage = formatVulkanErrorMessage(exprStr, result);
        }
        return result;
    }

#if defined(VULKAN_DEBUG_UTILS)

    void beginLabel(
        vk::CommandBuffer                        commandBuffer,
        const vk::detail::DispatchLoaderDynamic& dispatchLoader,
        const std::string&                       name,
        const glm::vec3&                         color = {1.0f, 1.0f, 1.0f}
    );

    void endLabel(vk::CommandBuffer commandBuffer, const vk::detail::DispatchLoaderDynamic& dispatchLoader);

#else

    void beginLabel(
        vk::CommandBuffer                        commandBuffer,
        const vk::detail::DispatchLoaderDynamic& dispatchLoader,
        const std::string&                       name,
        const glm::vec3&                         color
    );

    void endLabel(vk::CommandBuffer commandBuffer, const vk::detail::DispatchLoaderDynamic& dispatchLoader);

#endif
}

#define VK_ERROR_MESSAGE(expr, result) \
    VulkanDebugger::formatVulkanErrorMessage(#expr, result)

///////////////////////////////////////////////////////////////////////////////
///                            NON-LOGGING CALLS                            ///
///////////////////////////////////////////////////////////////////////////////

#define VK_CALL(expr, errorVar) \
    VulkanDebugger::checkVulkanResult(#expr, [&] { return expr; }, errorVar)

#define VK_CALL_VOID(expr, errorVar) \
    VulkanDebugger::checkVulkanResultVoid(#expr, expr, errorVar);

#define VK_TRY(expr, errorVar) \
    do { \
        std::string errorMessageTry_##__COUNTER__; \
        const auto returnValueTry_##__COUNTER__ = VK_CALL(expr, errorMessageTry_##__COUNTER__); \
        if (VulkanDebugger::getVulkanResult(returnValueTry_##__COUNTER__) != vk::Result::eSuccess) { \
            errorVar = errorMessageTry_##__COUNTER__; \
            return false; \
        } \
    } while (0)

#define VK_TRY_VOID(expr, errorVar) \
    do { \
        const auto returnValueTryVoidLog_##__COUNTER__ = VK_CALL_VOID(expr, errorVar); \
        if (returnValueTryVoidLog_##__COUNTER__ != vk::Result::eSuccess) { \
            return; \
        } \
    } while (0)

#define VK_CREATE(expr, var, errorVar) \
    do { \
        const auto returnValueCreate_##__COUNTER__ = VK_CALL(expr, errorVar); \
        if (returnValueCreate_##__COUNTER__.result != vk::Result::eSuccess) { \
            return false; \
        } \
        var = returnValueCreate_##__COUNTER__.value; \
    } while (0)

#define VK_CREATE_VOID(expr, var, errorVar) \
    do { \
        const auto returnValueCreateVoid_##__COUNTER__ = VK_CALL(expr, errorVar); \
        if (returnValueCreateVoid_##__COUNTER__.result != vk::Result::eSuccess) { \
            return; \
        } \
        var = returnValueCreateVoid_##__COUNTER__.value; \
    } while (0)

///////////////////////////////////////////////////////////////////////////////
///                              LOGGING CALLS                              ///
///////////////////////////////////////////////////////////////////////////////

#define VK_CALL_LOG(expr, level) \
    ([&] { \
        std::string errorMessageCallLog_##__COUNTER__; \
        const auto returnValueCallLog_##__COUNTER__ = VK_CALL(expr, errorMessageCallLog_##__COUNTER__); \
        if (VulkanDebugger::getVulkanResult(returnValueCallLog_##__COUNTER__) != vk::Result::eSuccess) { \
            Logger::log(level, errorMessageCallLog_##__COUNTER__); \
        } \
        return returnValueCallLog_##__COUNTER__; \
    }())

#define VK_TRY_LOG(expr, level) \
    do { \
        std::string errorMessageTryLog_##__COUNTER__; \
        const auto returnValueTryLog_##__COUNTER__ = VK_CALL(expr, errorMessageTryLog_##__COUNTER__); \
        if (VulkanDebugger::getVulkanResult(returnValueTryLog_##__COUNTER__) != vk::Result::eSuccess) { \
            Logger::log(level, errorMessageTryLog_##__COUNTER__); \
            return false; \
        } \
    } while (0)

#define VK_TRY_VOID_LOG(expr, level) \
    do { \
        std::string errorMessageTryVoidLog_##__COUNTER__; \
        const auto returnValueTryVoidLog_##__COUNTER__ = VK_CALL_VOID(expr, errorMessageTryVoidLog_##__COUNTER__); \
        if (returnValueTryVoidLog_##__COUNTER__ != vk::Result::eSuccess) { \
            Logger::log(level, errorMessageTryVoidLog_##__COUNTER__); \
            return; \
        } \
    } while (0)

#define VK_CREATE_LOG(expr, var, level) \
    do { \
        std::string errorMessageCreateLog_##__COUNTER__; \
        const auto returnValueCreateLog_##__COUNTER__ = VK_CALL(expr, errorMessageCreateLog_##__COUNTER__); \
        if (returnValueCreateLog_##__COUNTER__.result != vk::Result::eSuccess) { \
            Logger::log(level, errorMessageCreateLog_##__COUNTER__); \
            return false; \
        } \
        var = returnValueCreateLog_##__COUNTER__.value; \
    } while (0)

#define VK_CREATE_VOID_LOG(expr, var, level) \
    do { \
        std::string errorMessageCreateVoidLog_##__COUNTER__; \
        const auto returnValueCreateVoidLog_##__COUNTER__ = VK_CALL(expr, errorMessageCreateVoidLog_##__COUNTER__); \
        if (returnValueCreateVoidLog_##__COUNTER__.result != vk::Result::eSuccess) { \
            Logger::log(level, errorMessageCreateVoidLog_##__COUNTER__); \
            return; \
        } \
        var = returnValueCreateVoidLog_##__COUNTER__.value; \
    } while (0)
