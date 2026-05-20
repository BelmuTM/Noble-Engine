#pragma once

#include "core/debug/ErrorHandling.h"

#include <string>

#include <glm/vec3.hpp>
#include <vulkan/vulkan.hpp>

#include "core/debug/Logger.h"

#ifdef FAIL
#define VK_FAIL(msg) \
    FAIL(msg, "Vulkan")
#endif

namespace VulkanDebugger {
    std::string formatVulkanErrorMessage(const std::string& expression, vk::Result result);

    // From C-style to HPP-style
    inline vk::Result extractVkResult(const vk::Result result) {
        return result;
    }

    inline vk::Result extractVkResult(VkResult result) {
        return static_cast<vk::Result>(result);
    }

    template<typename T>
    vk::Result extractVkResult(const vk::ResultValue<T>& resultValue) {
        return resultValue.result;
    }

    inline Unexpected makeVkFailure(const char* exprStr, const vk::Result result) {
        return VK_FAIL(formatVulkanErrorMessage(exprStr, result));
    }

    template<typename ResultType>
    Expected<void> VK_CHECK(const char* exprStr, const ResultType& result) {
        const vk::Result res = extractVkResult(result);
        if (res != vk::Result::eSuccess) {
            return makeVkFailure(exprStr, res);
        }
        return {};
    }

    template<typename ResultType>
    vk::Result VK_CHECK_RESULT(const char* exprStr, const ResultType& result, Failure* outFailure = nullptr) {
        const vk::Result res = extractVkResult(result);
        if (res != vk::Result::eSuccess) {
            if (outFailure) {
                *outFailure = makeVkFailure(exprStr, res).failure;
            }
        }
        return res;
    }

    template<typename ReturnValue>
    vk::ResultValueType<ReturnValue> VK_CHECK_RESULT(
        const char* exprStr, const vk::ResultValueType<ReturnValue>& resultValue, Failure* outFailure = nullptr
    ) {
        const vk::Result res = extractVkResult(resultValue.result);
        if (res != vk::Result::eSuccess) {
            if (outFailure) {
                *outFailure = makeVkFailure(exprStr, res).failure;
            }
        }
        return resultValue;
    }

    template<typename ResultType>
    void VK_CHECK_LOG(const char* exprStr, const ResultType& result) {
        const vk::Result res = extractVkResult(result);
        if (res != vk::Result::eSuccess) {
            Logger::error(formatVulkanErrorMessage(exprStr, res));
        }
    }

    template<typename T>
    Expected<T> VK_FROM_RESULT(const char* exprStr, const vk::ResultValue<T>& resultValue) {
        const vk::Result res = extractVkResult(resultValue.result);
        if (res != vk::Result::eSuccess) {
            return makeVkFailure(exprStr, res);
        }
        return Expected<T>(std::move(resultValue.value));
    }

#ifdef VULKAN_DEBUG_UTILS

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

#define VK_EXPECT(expr) \
    VulkanDebugger::VK_FROM_RESULT(#expr, (expr))

#define VK_FIRE_AND_FORGET(expr) \
    VulkanDebugger::VK_CHECK_LOG(#expr, (expr))

#define VK_RESULT(expr, failurePtr)                                        \
    ([&]() {                                                               \
        auto _r = (expr);                                                  \
        if (VulkanDebugger::extractVkResult(_r) != vk::Result::eSuccess) { \
            if (failurePtr)                                                \
                *failurePtr = VulkanDebugger::makeVkFailure(#expr,         \
                    VulkanDebugger::extractVkResult(_r)).failure;          \
        }                                                                  \
        return _r;                                                         \
    }())

#define VK_TRY(expr)                                                \
    do {                                                            \
        auto _resultTry_ = VulkanDebugger::VK_CHECK(#expr, (expr)); \
        if (_resultTry_.failed())                                   \
            return Unexpected{std::move(_resultTry_.failure())};    \
    } while (0)

#define VK_CREATE(varOut, expr)                                     \
    do {                                                            \
        auto _resultCreate_ = VK_EXPECT(expr);                      \
        if (!_resultCreate_)                                        \
            return Unexpected{std::move(_resultCreate_.failure())}; \
        varOut = std::move(_resultCreate_.value());                 \
    } while (0)
