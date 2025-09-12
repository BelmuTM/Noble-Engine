#pragma once
#ifndef BAZARENGINE_VULKANSURFACE_H
#define BAZARENGINE_VULKANSURFACE_H

#include "core/platform/Platform.h"

#include <memory>
#include <string>

#include <vulkan/vulkan.h>

class VulkanSurface {
public:
    VulkanSurface() = default;
    ~VulkanSurface();

    // Implicit conversion operator
    operator VkSurfaceKHR() const { return surface; }

    VulkanSurface(const VulkanSurface&)            = delete;
    VulkanSurface& operator=(const VulkanSurface&) = delete;
    VulkanSurface(VulkanSurface&&)                 = delete;
    VulkanSurface& operator=(VulkanSurface&&)      = delete;

    [[nodiscard]] bool create(
        VkInstance* instance, const Platform::Window& window, std::string& errorMessage
    ) noexcept;
    void destroy() noexcept;

private:
    const Platform::Window* _window = nullptr;

    VkInstance*  _instance = nullptr;
    VkSurfaceKHR surface   = VK_NULL_HANDLE;

    bool createSurface(std::string& errorMessage);
};

#endif //BAZARENGINE_VULKANSURFACE_H
