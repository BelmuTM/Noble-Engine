#include "VulkanSurface.h"

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif

VulkanSurface::~VulkanSurface() {
    destroy();
}

bool VulkanSurface::create(VkInstance* _instance, const WindowHandle& _window, std::string& errorMessage) noexcept {
    window   = _window;
    instance = _instance;

    if (!createSurface(errorMessage)) return false;
    return true;
}

void VulkanSurface::destroy() noexcept {
    if (surface != VK_NULL_HANDLE && instance) {
        vkDestroySurfaceKHR(*instance, surface, nullptr);
        window  = nullptr;
        surface = VK_NULL_HANDLE;
    }
}

bool VulkanSurface::createSurface(std::string& errorMessage) {
    if (!window) {
        errorMessage = "Failed to create Vulkan window surface: window handle is null";
        return false;
    }

#if defined(_WIN32) || defined(_WIN64)

    const VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{
        .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = GetModuleHandle(nullptr),
        .hwnd      = static_cast<HWND>(window)
    };

    const VkResult result = vkCreateWin32SurfaceKHR(*instance, &surfaceCreateInfo, nullptr, &surface);
    if (result != VK_SUCCESS) {
        errorMessage = "Failed to create Vulkan window surface";
        return false;
    }

    return true;

#elif defined(__linux__)
    // TO-DO
#endif
}
