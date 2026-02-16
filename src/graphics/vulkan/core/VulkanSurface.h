#pragma once

#include "core/platform/Window.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include <string>

class VulkanSurface {
public:
    VulkanSurface()  = default;
    ~VulkanSurface() = default;

    VulkanSurface(const VulkanSurface&)            = delete;
    VulkanSurface& operator=(const VulkanSurface&) = delete;

    VulkanSurface(VulkanSurface&&)            = delete;
    VulkanSurface& operator=(VulkanSurface&&) = delete;

    [[nodiscard]] bool create(const vk::Instance& instance, const Window& window, std::string& errorMessage) noexcept;

    void destroy() noexcept;

    [[nodiscard]] vk::SurfaceKHR handle() const noexcept { return _surface; }

private:
    bool createSurface(std::string& errorMessage);

    const Window* _window = nullptr;

    vk::Instance   _instance{};
    vk::SurfaceKHR _surface{};
};
