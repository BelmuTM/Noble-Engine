#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "VulkanCapabilities.h"

#include <string>
#include <vector>

class VulkanInstance {
public:
    VulkanInstance()  = default;
    ~VulkanInstance() = default;

    VulkanInstance(const VulkanInstance&)            = delete;
    VulkanInstance& operator=(const VulkanInstance&) = delete;

    VulkanInstance(VulkanInstance&&)            = delete;
    VulkanInstance& operator=(VulkanInstance&&) = delete;

    [[nodiscard]] bool create(const VulkanCapabilities& capabilities, std::string& errorMessage) noexcept;

    void destroy() noexcept;

    [[nodiscard]] vk::Instance handle() const noexcept { return _instance; }

private:
    static std::vector<const char*> getRequiredExtensions();

    bool createInstance(std::string& errorMessage);

#ifdef VULKAN_VALIDATION_LAYERS_ENABLED

    bool setupDebugMessenger(std::string& errorMessage);

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT      severity,
        vk::DebugUtilsMessageTypeFlagsEXT             type,
        const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void*
    );

#endif

    const VulkanCapabilities* _capabilities = nullptr;

    vk::Instance               _instance{};
    vk::DebugUtilsMessengerEXT _debugMessenger{};

    vk::detail::DispatchLoaderDynamic _dldi{};
};
