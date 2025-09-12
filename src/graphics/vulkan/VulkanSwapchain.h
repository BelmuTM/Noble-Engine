#pragma once
#ifndef BAZARENGINE_VULKANSWAPCHAIN_H
#define BAZARENGINE_VULKANSWAPCHAIN_H

#include "core/platform/Platform.h"
#include "VulkanDevice.h"

#include <string>
#include <vector>

#include <vulkan/vulkan_core.h>

class VulkanSwapchain {
public:
    VulkanSwapchain() = default;
    ~VulkanSwapchain();

    VulkanSwapchain(const VulkanSwapchain&)            = delete;
    VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;
    VulkanSwapchain(VulkanSwapchain&&)                 = delete;
    VulkanSwapchain& operator=(VulkanSwapchain&&)      = delete;

    [[nodiscard]] bool create(
        const Platform::Window& window, const VulkanDevice& device, VkSurfaceKHR surface, std::string& errorMessage
    ) noexcept;
    void destroy() noexcept;

private:
    const VulkanDevice*     _device = nullptr;
    const Platform::Window* _window = nullptr;

    VkSwapchainKHR           swapchain = VK_NULL_HANDLE;
    std::vector<VkImage>     swapchainImages;
    std::vector<VkImageView> swapchainImageViews;

    VkFormat   swapchainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D swapchainExtent{};

    bool createSwapchain(VkSurfaceKHR surface, std::string& errorMessage);

    struct SwapchainSupportInfo {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;
    };

    static SwapchainSupportInfo querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR _surface);

    static VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    static VkPresentModeKHR   choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    [[nodiscard]] VkExtent2D  chooseSwapExtent2D(const VkSurfaceCapabilitiesKHR& capabilities) const;
};

#endif //BAZARENGINE_VULKANSWAPCHAIN_H
