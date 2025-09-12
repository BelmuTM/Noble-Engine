#pragma once
#ifndef BAZARENGINE_VULKANINSTANCE_H
#define BAZARENGINE_VULKANINSTANCE_H

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

class VulkanInstance {
public:
    VulkanInstance() = default;
    ~VulkanInstance();

    // Implicit conversion operator
    operator VkInstance() const { return instance; }

    VulkanInstance(const VulkanInstance&)            = delete;
    VulkanInstance& operator=(const VulkanInstance&) = delete;
    VulkanInstance(VulkanInstance&&)                 = delete;
    VulkanInstance& operator=(VulkanInstance&&)      = delete;

    [[nodiscard]] bool create(std::string& errorMessage) noexcept;
    void               destroy() noexcept;

    VkInstance&       getVkInstance()       { return instance; }
    VkInstance const& getVkInstance() const { return instance; }

private:
    VkInstance               instance       = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

    static std::vector<const char*> getRequiredExtensions();

    bool createInstance(std::string& errorMessage);
    void setupDebugMessenger();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT      severity,
        VkDebugUtilsMessageTypeFlagsEXT             type,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void*
    );

    static VkResult createDebugUtilsMessengerEXT(
        VkInstance                                instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks*              pAllocator,
        VkDebugUtilsMessengerEXT*                 pMessenger
    );

    static void destroyDebugUtilsMessengerEXT(
        VkInstance                   instance,
        VkDebugUtilsMessengerEXT     messenger,
        const VkAllocationCallbacks* pAllocator
    );
};

#endif //BAZARENGINE_VULKANINSTANCE_H
