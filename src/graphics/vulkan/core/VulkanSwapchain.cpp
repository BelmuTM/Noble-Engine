#include "VulkanSwapchain.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

#include "core/debug/Logger.h"

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
            imageView = VK_NULL_HANDLE;
        }
    }

    swapchainImageViews.clear();

    if (swapchain) {
        _device->getLogicalDevice().destroySwapchainKHR(swapchain);
        swapchain = VK_NULL_HANDLE;
    }
}

bool VulkanSwapchain::recreate(const vk::SurfaceKHR surface, std::string& errorMessage) {
    VK_CALL(_device->getLogicalDevice().waitIdle(), errorMessage);

    destroy();

    if (!create(*_window, *_device, surface, errorMessage)) return false;
    return true;
}

VulkanSwapchain::SwapchainSupportInfo VulkanSwapchain::querySwapchainSupport(
    const vk::PhysicalDevice device, const vk::SurfaceKHR _surface, std::string& errorMessage
) {
    const auto surfaceCapabilities = VK_CALL(device.getSurfaceCapabilitiesKHR(_surface), errorMessage);
    const auto surfaceFormats      = VK_CALL(device.getSurfaceFormatsKHR(_surface), errorMessage);
    const auto surfacePresentModes = VK_CALL(device.getSurfacePresentModesKHR(_surface), errorMessage);

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
    return vk::PresentModeKHR::eImmediate;
}

vk::Extent2D VulkanSwapchain::chooseSwapExtent2D(const vk::SurfaceCapabilitiesKHR& capabilities) const {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    int width, height;
    _window->getFramebufferSize(width, height);

    return {
        std::clamp<uint32_t>(width , capabilities.minImageExtent.width , capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
}

bool VulkanSwapchain::createSwapchain(const vk::SurfaceKHR surface, std::string& errorMessage) {
    errorMessage = "Failed to create Vulkan swapchain: ";

    if (!_window) {
        errorMessage += "window is null";
        return false;
    }

    int width, height;
    _window->getFramebufferSize(width, height);
    if (width == 0 || height == 0) {
        errorMessage += "window size is null";
        return false;
    }

    const auto [capabilities, formats, presentModes] =
        querySwapchainSupport(_device->getPhysicalDevice(), surface, errorMessage);

    const vk::SurfaceFormatKHR& surfaceFormat = chooseSurfaceFormat(formats);
    const vk::PresentModeKHR&   presentMode   = choosePresentMode(presentModes);
    const vk::Extent2D&         swapExtent    = chooseSwapExtent2D(capabilities);

    auto minImageCount = std::max(3u, capabilities.minImageCount);
         minImageCount = capabilities.maxImageCount > 0 && minImageCount > capabilities.maxImageCount
                             ? capabilities.maxImageCount
                             : minImageCount;

    uint32_t imageCount = capabilities.minImageCount + 1;

    if (capabilities.maxImageCount > 0) {
        imageCount = std::min(imageCount, capabilities.maxImageCount);
    }

    vk::SwapchainCreateInfoKHR swapchainInfo{};
    swapchainInfo
        .setFlags(vk::SwapchainCreateFlagsKHR{})
        .setSurface(surface)
        .setMinImageCount(minImageCount)
        .setImageFormat(surfaceFormat.format)
        .setImageColorSpace(surfaceFormat.colorSpace)
        .setImageExtent(swapExtent)
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setImageSharingMode(vk::SharingMode::eExclusive)
        .setPreTransform(capabilities.currentTransform)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setPresentMode(presentMode)
        .setClipped(vk::True)
        .setOldSwapchain(nullptr);

    auto [graphicsFamily, presentFamily] = _device->getQueueFamilyIndices();

    const uint32_t queueFamilyIndices[] = {graphicsFamily, presentFamily};

    if (graphicsFamily != presentFamily) {
        swapchainInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
        swapchainInfo.setQueueFamilyIndexCount(2);
        swapchainInfo.setQueueFamilyIndices(queueFamilyIndices);
    } else {
        swapchainInfo.setImageSharingMode(vk::SharingMode::eExclusive);
        swapchainInfo.setQueueFamilyIndexCount(0);
        swapchainInfo.setQueueFamilyIndices(nullptr);
    }

    const vk::Device& logicalDevice = _device->getLogicalDevice();

    VK_CREATE(logicalDevice.createSwapchainKHR(swapchainInfo), swapchain, errorMessage);

    swapchainImageFormat = surfaceFormat.format;
    swapchainExtent      = swapExtent;

    VK_CREATE(logicalDevice.getSwapchainImagesKHR(swapchain), swapchainImages, errorMessage);
    return true;
}

bool VulkanSwapchain::createImageViews(std::string& errorMessage) {
    const vk::Device& logicalDevice = _device->getLogicalDevice();

    swapchainImageViews.clear();
    swapchainImageViews.reserve(swapchainImages.size());

    vk::ImageViewCreateInfo imageViewInfo{};
    imageViewInfo
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(swapchainImageFormat)
        .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

    for (const auto swapchainImage : swapchainImages) {
        imageViewInfo.image = swapchainImage;

        const auto imageViewCreate = VK_CALL(logicalDevice.createImageView(imageViewInfo), errorMessage);
        if (imageViewCreate.result != vk::Result::eSuccess) return false;

        swapchainImageViews.emplace_back(imageViewCreate.value);
    }
    return true;
}
