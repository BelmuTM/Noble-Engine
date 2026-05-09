#include "VulkanSwapchain.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "core/debug/Logger.h"

#include <algorithm>
#include <limits>

Expected<void> VulkanSwapchain::create(
    const Window& window, const VulkanDevice& device, const vk::SurfaceKHR surface
) noexcept {
    _window = &window;
    _device = &device;

    TRY(createSwapchain(surface));
    TRY(createImageViews());

    createImages();

    return {};
}

void VulkanSwapchain::destroy() noexcept {
    if (!_device) return;

    _imageHandles.clear();

    for (auto& imageView : _imageViews) {
        if (imageView) {
            _device->getLogicalDevice().destroyImageView(imageView);
            imageView = VK_NULL_HANDLE;
        }
    }

    _imageViews.clear();

    if (_swapchain) {
        _device->getLogicalDevice().destroySwapchainKHR(_swapchain);
        _swapchain = VK_NULL_HANDLE;
    }
}

Expected<void> VulkanSwapchain::recreate(const vk::SurfaceKHR surface) {
    if (!_device) {
        return VK_FAIL("Failed to recreate swapchain: device is null.");
    }

    VK_TRY(_device->getLogicalDevice().waitIdle());

    destroy();

    TRY(create(*_window, *_device, surface));

    return {};
}

void VulkanSwapchain::createImages() {
    _images.resize(_imageHandles.size());

    for (std::uint32_t i = 0; i < _imageHandles.size(); i++) {
        if (!_images[i]) {
            _images[i] = std::make_unique<VulkanImage>();
        }

        _images[i]->setHandle(_imageHandles[i]);
        _images[i]->setImageView(_imageViews[i]);
        _images[i]->setFormat(_format);
        _images[i]->setExtent(vk::Extent3D{_extent, 1});
    }
}

Expected<VulkanSwapchain::SwapchainSupportInfo> VulkanSwapchain::querySwapchainSupport(
    const vk::PhysicalDevice device, const vk::SurfaceKHR surface
) {
    vk::SurfaceCapabilitiesKHR capabilities;
    VK_CREATE(capabilities, device.getSurfaceCapabilitiesKHR(surface));

    std::vector<vk::SurfaceFormatKHR> formats;
    VK_CREATE(formats, device.getSurfaceFormatsKHR(surface));

    std::vector<vk::PresentModeKHR> modes;
    VK_CREATE(modes, device.getSurfacePresentModesKHR(surface));

    return Expected<SwapchainSupportInfo>({capabilities, formats, modes});
}

vk::SurfaceFormatKHR VulkanSwapchain::chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format     == vk::Format::eB8G8R8A8Srgb &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }

    Logger::warning("Failed to find a supported RGBA8 sRGB surface format: falling back to the first available format.");

    return availableFormats.front();
}

vk::PresentModeKHR VulkanSwapchain::choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    if (availablePresentModes.empty()) {
        Logger::error("Failed to find available present modes.");
    }

    for (const auto& availableMode : availablePresentModes) {
        if (availableMode == vk::PresentModeKHR::eMailbox) {
            return availableMode;
        }
    }

    return vk::PresentModeKHR::eImmediate;
}

vk::Extent2D VulkanSwapchain::chooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const {
    if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    int width, height;
    _window->getFramebufferSize(width, height);

    return {
        std::clamp<std::uint32_t>(width , capabilities.minImageExtent.width , capabilities.maxImageExtent.width),
        std::clamp<std::uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
}

Expected<void> VulkanSwapchain::createSwapchain(const vk::SurfaceKHR surface) {
    if (!_window) {
        return VK_FAIL("Failed to create swapchain: window is null.");
    }

    int width, height;
    _window->getFramebufferSize(width, height);
    if (width == 0 || height == 0) {
        return VK_FAIL("Failed to create swapchain: window size is null.");
    }

    SwapchainSupportInfo supportInfo;
    VK_TRY_ASSIGN(supportInfo, querySwapchainSupport(_device->getPhysicalDevice(), surface));

    const vk::SurfaceFormatKHR& surfaceFormat = chooseSurfaceFormat(supportInfo.formats);
    const vk::PresentModeKHR&   presentMode   = choosePresentMode(supportInfo.presentModes);
    const vk::Extent2D&         extent        = chooseExtent(supportInfo.capabilities);

    std::uint32_t minImageCount = std::max(3u, supportInfo.capabilities.minImageCount);

    if (supportInfo.capabilities.maxImageCount > 0) {
        minImageCount = std::min(minImageCount, supportInfo.capabilities.maxImageCount);
    }

    vk::SwapchainCreateInfoKHR swapchainInfo{};
    swapchainInfo
        .setFlags(vk::SwapchainCreateFlagsKHR{})
        .setSurface(surface)
        .setMinImageCount(minImageCount)
        .setImageFormat(surfaceFormat.format)
        .setImageColorSpace(surfaceFormat.colorSpace)
        .setImageExtent(extent)
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setImageSharingMode(vk::SharingMode::eExclusive)
        .setPreTransform(supportInfo.capabilities.currentTransform)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setPresentMode(presentMode)
        .setClipped(vk::True)
        .setOldSwapchain(nullptr);

    auto [graphicsFamily, presentFamily] = _device->getQueueFamilyIndices();

    const std::uint32_t queueFamilyIndices[] = {graphicsFamily, presentFamily};

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

    VK_CREATE(_swapchain, logicalDevice.createSwapchainKHR(swapchainInfo));

    _format = surfaceFormat.format;
    _extent = extent;

    VK_CREATE(_imageHandles, logicalDevice.getSwapchainImagesKHR(_swapchain));

    return {};
}

Expected<void> VulkanSwapchain::createImageViews() {
    const vk::Device& logicalDevice = _device->getLogicalDevice();

    _imageViews.clear();
    _imageViews.reserve(_imageHandles.size());

    vk::ImageViewCreateInfo imageViewInfo{};
    imageViewInfo
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(_format)
        .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

    for (const auto swapchainImage : _imageHandles) {
        imageViewInfo.image = swapchainImage;

        vk::ImageView imageView;
        VK_CREATE(imageView, logicalDevice.createImageView(imageViewInfo));

        _imageViews.push_back(std::move(imageView));
    }

    return {};
}
