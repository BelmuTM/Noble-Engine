#pragma once

#include "core/platform/Window.h"

#include "graphics/vulkan/common/VulkanEntityOwner.h"

#include "graphics/vulkan/core/VulkanCapabilities.h"
#include "graphics/vulkan/core/VulkanInstance.h"
#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/core/VulkanSurface.h"
#include "graphics/vulkan/core/VulkanSwapchain.h"

class VulkanContext final : public VulkanEntityOwner<VulkanContext> {
public:
    VulkanContext()  = default;
    ~VulkanContext() = default;

    VulkanContext(const VulkanContext&)            = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

    VulkanContext(VulkanContext&&)            = delete;
    VulkanContext& operator=(VulkanContext&&) = delete;

    bool create(const Window& window, std::string& errorMessage);

    void destroy();

    VulkanInstance&  getInstance()  { return instance; }
    VulkanDevice&    getDevice()    { return device; }
    VulkanSurface&   getSurface()   { return surface; }
    VulkanSwapchain& getSwapchain() { return swapchain; }

    [[nodiscard]] const VulkanInstance&  getInstance()  const { return instance; }
    [[nodiscard]] const VulkanDevice&    getDevice()    const { return device; }
    [[nodiscard]] const VulkanSurface&   getSurface()   const { return surface; }
    [[nodiscard]] const VulkanSwapchain& getSwapchain() const { return swapchain; }

private:
    VulkanCapabilities capabilities{};
    VulkanInstance     instance{};
    VulkanSurface      surface{};
    VulkanDevice       device{};
    VulkanSwapchain    swapchain{};
};
