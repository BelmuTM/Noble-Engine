#include "VulkanSwapchain.h"
#include "VulkanLogger.h"

#include "core/debug/Logger.h"

#include <algorithm>
#include <limits>

VulkanSwapchain::~VulkanSwapchain() {
    destroy();
}

bool VulkanSwapchain::create(
    const Platform::Window& window, const VulkanDevice& device, VkSurfaceKHR surface, std::string& errorMessage
) noexcept {
    _window = &window;
    _device = &device;

    if (!createSwapchain(surface, errorMessage)) return false;
    return true;
}

void VulkanSwapchain::destroy() noexcept {
    if (swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(_device->getLogicalDevice(), swapchain, nullptr);
        swapchain = VK_NULL_HANDLE;
    }
}

VulkanSwapchain::SwapchainSupportInfo VulkanSwapchain::querySwapchainSupport(
    const VkPhysicalDevice device, const VkSurfaceKHR _surface
) {
    VkSurfaceCapabilitiesKHR surfaceCapabilitiesKHR{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &surfaceCapabilitiesKHR);

    uint32_t surfaceFormatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &surfaceFormatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &surfaceFormatCount, surfaceFormats.data());

    uint32_t surfacePresentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &surfacePresentModeCount, nullptr);
    std::vector<VkPresentModeKHR> surfacePresentModes(surfacePresentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &surfacePresentModeCount, surfacePresentModes.data());

    return {surfaceCapabilitiesKHR, surfaceFormats, surfacePresentModes};
}

VkSurfaceFormatKHR VulkanSwapchain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    Logger::warning("Failed to find a supported RGBA8 SRGB format: falling back to the first available format");
    return availableFormats.front();
}

VkPresentModeKHR VulkanSwapchain::choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    if (availablePresentModes.empty()) {
        Logger::error("Failed to find available Vulkan present modes");
    }

    for (const auto& availableMode : availablePresentModes) {
        if (availableMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availableMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapchain::chooseSwapExtent2D(const VkSurfaceCapabilitiesKHR& capabilities) const {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    int width, height;
    _window->getFrameBufferSize(width, height);

    return {
        std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
}

bool VulkanSwapchain::createSwapchain(VkSurfaceKHR surface, std::string& errorMessage) {
    const auto [capabilities, formats, presentModes] = querySwapchainSupport(_device->getPhysicalDevice(), surface);

    const VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(formats);
    const VkPresentModeKHR   presentMode   = choosePresentMode(presentModes);
    const VkExtent2D         swapExtent    = chooseSwapExtent2D(capabilities);

    auto minImageCount = std::max(3u, capabilities.minImageCount);
         minImageCount = capabilities.maxImageCount > 0 && minImageCount > capabilities.maxImageCount ? capabilities.maxImageCount : minImageCount;

    uint32_t imageCount = capabilities.minImageCount + 1;

    if (capabilities.maxImageCount > 0) {
        imageCount = std::min(imageCount, capabilities.maxImageCount);
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo{
        .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .flags            = VkSwapchainCreateFlagsKHR(),
        .surface          = surface,
        .minImageCount    = minImageCount,
        .imageFormat      = surfaceFormat.format,
        .imageColorSpace  = surfaceFormat.colorSpace,
        .imageExtent      = swapExtent,
        .imageArrayLayers = 1,
        .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform     = capabilities.currentTransform,
        .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode      = presentMode,
        .clipped          = VK_TRUE,
        .oldSwapchain     = VK_NULL_HANDLE,
    };

    auto [graphicsFamily, presentFamily] = _device->getQueueFamilyIndices();

    const uint32_t queueFamilyIndices[] = {graphicsFamily, presentFamily};

    if (graphicsFamily != presentFamily) {
        swapchainCreateInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices   = queueFamilyIndices;
    } else {
        swapchainCreateInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices   = nullptr;
    }

    const VkDevice& logicalDevice = _device->getLogicalDevice();

    const VkResult result = vkCreateSwapchainKHR(logicalDevice, &swapchainCreateInfo, nullptr, &swapchain);
    if (result != VK_SUCCESS) {
        errorMessage = VK_ERROR_MESSAGE(vkCreateSwapchainKHR, result);
        return false;
    }

    // Retrieve swapchain images
    uint32_t swapchainImageCount = 0;
    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &swapchainImageCount, nullptr);
    swapchainImages = std::vector<VkImage>(swapchainImageCount);
    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &swapchainImageCount, swapchainImages.data());

    swapchainImageFormat = surfaceFormat.format;
    swapchainExtent      = swapExtent;
    return true;
}
