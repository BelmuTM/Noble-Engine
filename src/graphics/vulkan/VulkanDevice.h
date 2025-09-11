#pragma once
#ifndef BAZARENGINE_VULKANDEVICE_H
#define BAZARENGINE_VULKANDEVICE_H

#include <string>

#include <vulkan/vulkan.h>

class VulkanDevice {
public:
    VulkanDevice() = default;
    ~VulkanDevice();

    VulkanDevice(const VulkanDevice&)            = delete;
    VulkanDevice& operator=(const VulkanDevice&) = delete;
    VulkanDevice(VulkanDevice&&)                 = delete;
    VulkanDevice& operator=(VulkanDevice&&)      = delete;

    [[nodiscard]] bool create(VkInstance instance, VkSurfaceKHR surface, std::string& errorMessage) noexcept;
    void               destroy() noexcept;

private:
    struct QueueFamilyIndices {
        uint32_t graphicsFamily = UINT32_MAX;
        uint32_t presentFamily  = UINT32_MAX;

        bool isComplete() const {
            return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX;
        }
    };

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice         logicalDevice  = VK_NULL_HANDLE;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue  = VK_NULL_HANDLE;

    static bool isPhysicalDeviceSuitable(VkPhysicalDevice device);
    bool        pickPhysicalDevice(VkInstance instance, std::string& errorMessage);

    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
    bool                      createLogicalDevice(VkSurfaceKHR surface, std::string& errorMessage);
};

#endif //BAZARENGINE_VULKANDEVICE_H
