#include "VulkanSurface.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

bool VulkanSurface::create(
    const vk::Instance& instance, const Platform::Window& window, std::string& errorMessage
) noexcept {
    _window   = &window;
    _instance = instance;

    if (!createSurface(errorMessage)) return false;

    return true;
}

void VulkanSurface::destroy() noexcept {
    if (_surface && _instance) {
        _instance.destroySurfaceKHR(_surface);
        _surface = VK_NULL_HANDLE;
    }
    _window   = nullptr;
    _instance = VK_NULL_HANDLE;
}

bool VulkanSurface::createSurface(std::string& errorMessage) {
    if (!_window) {
        errorMessage = "Failed to create Vulkan window surface: window handle is null";
        return false;
    }

    VkSurfaceKHR surface;
    const VkResult create = glfwCreateWindowSurface(_instance, _window->handle(), nullptr, &surface);

    if (static_cast<vk::Result>(create) != vk::Result::eSuccess) {
        errorMessage =
            VulkanDebugger::formatVulkanErrorMessage("glfwCreateWindowSurface", static_cast<vk::Result>(create));
    }

    _surface = vk::SurfaceKHR(surface);
    return true;
}
