#pragma once
#ifndef NOBLEENGINE_VULKANSURFACE_H
#define NOBLEENGINE_VULKANSURFACE_H

#include "core/platform/Window.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include <string>

class VulkanSurface {
public:
    VulkanSurface()  = default;
    ~VulkanSurface() = default;

    // Implicit conversion operator
    operator vk::SurfaceKHR() const noexcept { return _surface; }

    VulkanSurface(const VulkanSurface&)            = delete;
    VulkanSurface& operator=(const VulkanSurface&) = delete;

    VulkanSurface(VulkanSurface&&)            = delete;
    VulkanSurface& operator=(VulkanSurface&&) = delete;

    [[nodiscard]] bool create(const vk::Instance& instance, const Window& window, std::string& errorMessage) noexcept;

    void destroy() noexcept;

private:
    bool createSurface(std::string& errorMessage);

    const Window* _window = nullptr;

    vk::Instance   _instance{};
    vk::SurfaceKHR _surface{};
};

#endif //NOBLEENGINE_VULKANSURFACE_H
