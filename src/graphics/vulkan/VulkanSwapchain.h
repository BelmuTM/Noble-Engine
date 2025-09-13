#pragma once
#ifndef BAZARENGINE_VULKANSWAPCHAIN_H
#define BAZARENGINE_VULKANSWAPCHAIN_H

#include "core/platform/Platform.h"
#include "VulkanDevice.h"

#include <string>
#include <vector>

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

class VulkanSwapchain {
public:
    VulkanSwapchain()  = default;
    ~VulkanSwapchain() = default;

    VulkanSwapchain(const VulkanSwapchain&)            = delete;
    VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;
    VulkanSwapchain(VulkanSwapchain&&)                 = delete;
    VulkanSwapchain& operator=(VulkanSwapchain&&)      = delete;

    [[nodiscard]] bool create(
        const Platform::Window& window, const VulkanDevice& device, vk::SurfaceKHR surface, std::string& errorMessage
    ) noexcept;
    void destroy() noexcept;

private:
    struct SwapchainSupportInfo {
        vk::SurfaceCapabilitiesKHR        capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR>   presentModes;
    };

    const VulkanDevice*     _device = nullptr;
    const Platform::Window* _window = nullptr;

    vk::SwapchainKHR           swapchain{};
    std::vector<vk::Image>     swapchainImages;
    std::vector<vk::ImageView> swapchainImageViews;

    vk::Format   swapchainImageFormat = vk::Format::eUndefined;
    vk::Extent2D swapchainExtent{};

    bool createSwapchain(vk::SurfaceKHR surface, std::string& errorMessage);

    bool createImageViews(std::string& errorMessage);

    static SwapchainSupportInfo querySwapchainSupport(vk::PhysicalDevice device, vk::SurfaceKHR _surface);

    static vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    static vk::PresentModeKHR   choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    [[nodiscard]] vk::Extent2D  chooseSwapExtent2D(const vk::SurfaceCapabilitiesKHR& capabilities) const;
};

#endif //BAZARENGINE_VULKANSWAPCHAIN_H
