#include "VulkanSurface.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

Expected<void> VulkanSurface::create(const vk::Instance& instance, const Window& window) noexcept {
    _window   = &window;
    _instance = instance;

    TRY(createSurface());

    return {};
}

void VulkanSurface::destroy() noexcept {
    if (_surface && _instance) {
        _instance.destroySurfaceKHR(_surface);
        _surface = VK_NULL_HANDLE;
    }

    _window   = nullptr;
    _instance = VK_NULL_HANDLE;
}

Expected<void> VulkanSurface::createSurface() {
    if (!_window) {
        return VK_FAIL("Failed to create window surface: window handle is null.");
    }

    VkSurfaceKHR surface;
    VK_TRY(glfwCreateWindowSurface(_instance, _window->handle(), nullptr, &surface));

    _surface = vk::SurfaceKHR(surface);

    return {};
}
