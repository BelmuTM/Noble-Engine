#include "VulkanSwapchain.h"
#include "VulkanDebugger.h"

#include "core/debug/Logger.h"

#include <algorithm>
#include <limits>

bool VulkanSwapchain::create(
    const Platform::Window& window, const VulkanDevice& device, const vk::SurfaceKHR surface, std::string& errorMessage
) noexcept {
    _window = &window;
    _device = &device;

    if (!createSwapchain(surface, errorMessage)) return false;
    if (!createImageViews(errorMessage))         return false;
    return true;
}

void VulkanSwapchain::destroy() noexcept {
    if (!_device) return;

    for (auto& imageView : swapchainImageViews) {
        if (imageView) {
            _device->getLogicalDevice().destroyImageView(imageView);
            imageView = nullptr;
        }
    }

    swapchainImageViews.clear();

    if (swapchain) {
        _device->getLogicalDevice().destroySwapchainKHR(swapchain);
        swapchain = nullptr;
    }
}

VulkanSwapchain::SwapchainSupportInfo VulkanSwapchain::querySwapchainSupport(
    const vk::PhysicalDevice device, const vk::SurfaceKHR _surface
) {
    std::string errorMessage;
    const auto surfaceCapabilities = VK_CHECK_RESULT(device.getSurfaceCapabilitiesKHR(_surface), errorMessage);
    if (surfaceCapabilities.result != vk::Result::eSuccess) {
        Logger::error(errorMessage);
    }

    const auto surfaceFormats = VK_CHECK_RESULT(device.getSurfaceFormatsKHR(_surface), errorMessage);
    if (surfaceFormats.result != vk::Result::eSuccess) {
        Logger::error(errorMessage);
    }

    const auto surfacePresentModes = VK_CHECK_RESULT(device.getSurfacePresentModesKHR(_surface), errorMessage);
    if (surfacePresentModes.result != vk::Result::eSuccess) {
        Logger::error(errorMessage);
    }

    return {surfaceCapabilities.value, surfaceFormats.value, surfacePresentModes.value};
}

vk::SurfaceFormatKHR VulkanSwapchain::chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format     == vk::Format::eB8G8R8A8Srgb &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }
    Logger::warning("Failed to find a supported RGBA8 SRGB format: falling back to the first available format");
    return availableFormats.front();
}

vk::PresentModeKHR VulkanSwapchain::choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    if (availablePresentModes.empty()) {
        Logger::error("Failed to find available Vulkan present modes");
    }

    for (const auto& availableMode : availablePresentModes) {
        if (availableMode == vk::PresentModeKHR::eMailbox) {
            return availableMode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkanSwapchain::chooseSwapExtent2D(const vk::SurfaceCapabilitiesKHR& capabilities) const {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    int width, height;
    _window->getFrameBufferSize(width, height);

    return {
        std::clamp<uint32_t>(width , capabilities.minImageExtent.width , capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
}

bool VulkanSwapchain::createSwapchain(const vk::SurfaceKHR surface, std::string& errorMessage) {
    const auto [capabilities, formats, presentModes] = querySwapchainSupport(_device->getPhysicalDevice(), surface);

    const vk::SurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(formats);
    const vk::PresentModeKHR   presentMode   = choosePresentMode(presentModes);
    const vk::Extent2D         swapExtent    = chooseSwapExtent2D(capabilities);

    auto minImageCount = std::max(3u, capabilities.minImageCount);
         minImageCount = capabilities.maxImageCount > 0 && minImageCount > capabilities.maxImageCount
                             ? capabilities.maxImageCount
                             : minImageCount;

    uint32_t imageCount = capabilities.minImageCount + 1;

    if (capabilities.maxImageCount > 0) {
        imageCount = std::min(imageCount, capabilities.maxImageCount);
    }

    vk::SwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.flags            = vk::SwapchainCreateFlagsKHR();
    swapchainCreateInfo.surface          = surface;
    swapchainCreateInfo.minImageCount    = minImageCount;
    swapchainCreateInfo.imageFormat      = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace  = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent      = swapExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;
    swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
    swapchainCreateInfo.preTransform     = capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapchainCreateInfo.presentMode      = presentMode;
    swapchainCreateInfo.clipped          = vk::True;
    swapchainCreateInfo.oldSwapchain     = nullptr;

    auto [graphicsFamily, presentFamily] = _device->getQueueFamilyIndices();

    const uint32_t queueFamilyIndices[] = {graphicsFamily, presentFamily};

    if (graphicsFamily != presentFamily) {
        swapchainCreateInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices   = queueFamilyIndices;
    } else {
        swapchainCreateInfo.imageSharingMode      = vk::SharingMode::eExclusive;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices   = nullptr;
    }

    const vk::Device logicalDevice = _device->getLogicalDevice();

    const auto swapchainCreate = VK_CHECK_RESULT(logicalDevice.createSwapchainKHR(swapchainCreateInfo), errorMessage);
    if (swapchainCreate.result != vk::Result::eSuccess) {
        return false;
    }
    swapchain = swapchainCreate.value;

    swapchainImageFormat = surfaceFormat.format;
    swapchainExtent      = swapExtent;

    const auto availableSwapchainImages = VK_CHECK_RESULT(logicalDevice.getSwapchainImagesKHR(swapchain), errorMessage);
    if (availableSwapchainImages.result != vk::Result::eSuccess) {
        return false;
    }
    swapchainImages = availableSwapchainImages.value;

    return true;
}

bool VulkanSwapchain::createImageViews(std::string& errorMessage) {
    swapchainImageViews.clear();

    vk::ImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.viewType         = vk::ImageViewType::e2D;
    imageViewCreateInfo.format           = swapchainImageFormat;
    imageViewCreateInfo.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

    const vk::Device logicalDevice = _device->getLogicalDevice();

    for (size_t i = 0; i < swapchainImages.size(); i++) {
        imageViewCreateInfo.image = swapchainImages[i];

        const auto imageViewCreate = VK_CHECK_RESULT(logicalDevice.createImageView(imageViewCreateInfo), errorMessage);
        if (imageViewCreate.result != vk::Result::eSuccess) {
            return false;
        }
        swapchainImageViews.emplace_back(imageViewCreate.value);
    }
    return true;
}
