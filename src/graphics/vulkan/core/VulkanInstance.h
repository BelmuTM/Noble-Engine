#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/core/VulkanCapabilities.h"

#include <vector>

class VulkanInstance {
public:
    VulkanInstance()  = default;
    ~VulkanInstance() = default;

    VulkanInstance(const VulkanInstance&)            = delete;
    VulkanInstance& operator=(const VulkanInstance&) = delete;

    VulkanInstance(VulkanInstance&&)            = delete;
    VulkanInstance& operator=(VulkanInstance&&) = delete;

    [[nodiscard]] Expected<void> create(const VulkanCapabilities& capabilities) noexcept;

    void destroy() noexcept;

    [[nodiscard]] vk::Instance handle() const noexcept { return _instance; }

private:
    static std::vector<const char*> getRequiredExtensions();

    Expected<void> createInstance();

#ifdef VULKAN_VALIDATION_LAYERS_ENABLED

    Expected<void> setupDebugMessenger();

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

    vk::detail::DispatchLoaderDynamic _dispatchLoader{};
};
