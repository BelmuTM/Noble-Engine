#include "VulkanSurface.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

bool VulkanSurface::create(
    const vk::Instance& instance, const Platform::Window& window, std::string& errorMessage
) noexcept {
    _window   = &window;
    _instance = instance;

    if (!createSurface(errorMessage)) { return false; }
    return true;
}

void VulkanSurface::destroy() noexcept {
    if (surface && _instance) {
        _instance.destroySurfaceKHR(surface);
        surface = VK_NULL_HANDLE;
    }
    _window   = nullptr;
    _instance = VK_NULL_HANDLE;
}

bool VulkanSurface::createSurface(std::string& errorMessage) {
    if (!_window) {
        errorMessage = "Failed to create Vulkan window surface: window handle is null";
        return false;
    }

#if defined(_WIN32) || defined(_WIN64)

    vk::Win32SurfaceCreateInfoKHR surfaceInfo{};
    surfaceInfo.hinstance = _window->hInstance();
    surfaceInfo.hwnd      = static_cast<HWND>(_window->nativeHandle());

    VK_CREATE(_instance.createWin32SurfaceKHR(surfaceInfo), surface, errorMessage);
    return true;

#elif defined(__linux__)
    // TO-DO
#endif
}
