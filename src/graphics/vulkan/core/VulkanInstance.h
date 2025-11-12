#pragma once
#ifndef NOBLEENGINE_VULKANINSTANCE_H
#define NOBLEENGINE_VULKANINSTANCE_H

#include "graphics/vulkan/common/VulkanHeader.h"
#include "VulkanCapabilities.h"

#include <string>
#include <vector>

class VulkanInstance {
public:
    VulkanInstance()  = default;
    ~VulkanInstance() = default;

    // Implicit conversion operator
    operator vk::Instance() const noexcept { return _instance; }

    VulkanInstance(const VulkanInstance&)            = delete;
    VulkanInstance& operator=(const VulkanInstance&) = delete;

    VulkanInstance(VulkanInstance&&)            = delete;
    VulkanInstance& operator=(VulkanInstance&&) = delete;

    [[nodiscard]] bool create(const VulkanCapabilities& capabilities, std::string& errorMessage) noexcept;

    void destroy() noexcept;

    //[[nodiscard]] vk::Instance&       getVkInstance()       { return instance; }
    //[[nodiscard]] vk::Instance const& getVkInstance() const { return instance; }

private:
    const VulkanCapabilities* _capabilities = nullptr;

    vk::Instance               _instance{};
    vk::DebugUtilsMessengerEXT _debugMessenger{};

    vk::detail::DispatchLoaderDynamic _dldi{};

    static std::vector<const char*> getRequiredExtensions();

    bool createInstance(std::string& errorMessage);

#ifdef VULKAN_DEBUG_UTILS

    bool setupDebugMessenger(std::string& errorMessage);

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT      severity,
        vk::DebugUtilsMessageTypeFlagsEXT             type,
        const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void*
    );

#endif
};

#endif //NOBLEENGINE_VULKANINSTANCE_H
