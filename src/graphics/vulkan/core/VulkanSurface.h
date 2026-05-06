#pragma once

#include "core/debug/ErrorHandling.h"
#include "core/platform/Window.h"

#include "graphics/vulkan/common/VulkanHeader.h"

class VulkanSurface {
public:
    VulkanSurface()  = default;
    ~VulkanSurface() = default;

    VulkanSurface(const VulkanSurface&)            = delete;
    VulkanSurface& operator=(const VulkanSurface&) = delete;

    VulkanSurface(VulkanSurface&&)            = delete;
    VulkanSurface& operator=(VulkanSurface&&) = delete;

    [[nodiscard]] Expected<void> create(const vk::Instance& instance, const Window& window) noexcept;

    void destroy() noexcept;

    [[nodiscard]] vk::SurfaceKHR handle() const noexcept { return _surface; }

private:
    Expected<void> createSurface();

    const Window* _window = nullptr;

    vk::Instance   _instance{};
    vk::SurfaceKHR _surface{};
};
