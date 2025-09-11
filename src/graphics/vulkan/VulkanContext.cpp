#include "VulkanContext.h"
#include "core/Engine.h"

#include <memory>

VulkanContext::~VulkanContext() {
    shutdown();
}

bool VulkanContext::init(const WindowHandle& _window) {
    std::string errorMessage;

    if (!instance.create(errorMessage)) {
        Engine::fatalExit(errorMessage);
    }

    if (!surface.create(&instance.getVkInstance(), _window, errorMessage)) {
        Engine::fatalExit(errorMessage);
    }

    if (!device.create(instance, surface, errorMessage)) {
        Engine::fatalExit(errorMessage);
    }
    return true;
}

void VulkanContext::shutdown() {
}

void VulkanContext::drawFrame() {
}

VulkanContext::SwapchainSupportInfo VulkanContext::querySwapchainSupport(
    const VkPhysicalDevice device, const VkSurfaceKHR _surface) {

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

void VulkanContext::createSwapchain() {
}
