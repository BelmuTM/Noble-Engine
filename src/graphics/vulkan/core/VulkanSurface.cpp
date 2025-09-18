#include "VulkanSurface.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

bool VulkanSurface::create(
    const vk::Instance* instance, const Platform::Window& window, std::string& errorMessage
) noexcept {
    _window   = &window;
    _instance = instance;

    if (!createSurface(errorMessage)) { return false; }
    return true;
}

void VulkanSurface::destroy() noexcept {
    if (surface && _instance) {
        _instance->destroySurfaceKHR(surface);
        _window = nullptr;
        surface = nullptr;
    }
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

    const auto win32SurfaceCreate = VK_CHECK_RESULT(_instance->createWin32SurfaceKHR(surfaceInfo), errorMessage);
    if (win32SurfaceCreate.result != vk::Result::eSuccess) return false;

    surface = win32SurfaceCreate.value;
    return true;

#elif defined(__linux__)
    // TO-DO
#endif
}
