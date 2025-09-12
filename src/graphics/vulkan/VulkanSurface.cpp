#include "VulkanSurface.h"
#include "VulkanLogger.h"

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif

VulkanSurface::~VulkanSurface() {
    destroy();
}

bool VulkanSurface::create(VkInstance* instance, const Platform::Window& window, std::string& errorMessage) noexcept {
    _window   = &window;
    _instance = instance;

    if (!createSurface(errorMessage)) return false;
    return true;
}

void VulkanSurface::destroy() noexcept {
    if (surface != VK_NULL_HANDLE && _instance) {
        vkDestroySurfaceKHR(*_instance, surface, nullptr);
        _window = nullptr;
        surface = VK_NULL_HANDLE;
    }
}

bool VulkanSurface::createSurface(std::string& errorMessage) {
    if (!_window) {
        errorMessage = "Failed to create Vulkan window surface: window handle is null";
        return false;
    }

#if defined(_WIN32) || defined(_WIN64)

    const VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{
        .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = _window->hInstance(),
        .hwnd      = static_cast<HWND>(_window->nativeHandle())
    };

    const VkResult result = vkCreateWin32SurfaceKHR(*_instance, &surfaceCreateInfo, nullptr, &surface);
    if (result != VK_SUCCESS) {
        errorMessage = VK_ERROR_MESSAGE(vkCreateWin32SurfaceKHR, result);
        return false;
    }

    return true;

#elif defined(__linux__)
    // TO-DO
#endif
}
