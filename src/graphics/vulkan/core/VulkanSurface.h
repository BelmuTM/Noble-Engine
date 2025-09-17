#pragma once
#ifndef NOBLEENGINE_VULKANSURFACE_H
#define NOBLEENGINE_VULKANSURFACE_H

#include "core/platform/Platform.h"

#include <string>

#define VULKAN_HPP_NO_EXCEPTIONS
#if defined(_WIN32) || defined(_WIN64)
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.hpp>

class VulkanSurface {
public:
    VulkanSurface() = default;
    ~VulkanSurface() = default;

    // Implicit conversion operator
    operator vk::SurfaceKHR() const { return surface; }

    VulkanSurface(const VulkanSurface&)            = delete;
    VulkanSurface& operator=(const VulkanSurface&) = delete;
    VulkanSurface(VulkanSurface&&)                 = delete;
    VulkanSurface& operator=(VulkanSurface&&)      = delete;

    [[nodiscard]] bool create(
        const vk::Instance* instance, const Platform::Window& window, std::string& errorMessage
    ) noexcept;
    void destroy() noexcept;

private:
    const Platform::Window* _window = nullptr;
    const vk::Instance*   _instance = nullptr;

    vk::SurfaceKHR surface{};

    bool createSurface(std::string& errorMessage);
};

#endif //NOBLEENGINE_VULKANSURFACE_H
