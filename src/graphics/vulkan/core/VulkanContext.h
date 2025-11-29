#pragma once
#ifndef NOBLEENGINE_VULKANCONTEXT_H
#define NOBLEENGINE_VULKANCONTEXT_H

#include "graphics/vulkan/common/VulkanEntityOwner.h"

#include "VulkanCapabilities.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"

class VulkanContext final : public VulkanEntityOwner<VulkanContext> {
public:
    VulkanContext() = default;
    ~VulkanContext();

    VulkanContext(const VulkanContext&)            = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

    VulkanContext(VulkanContext&&)            = delete;
    VulkanContext& operator=(VulkanContext&&) = delete;

    bool create(const Platform::Window& window, std::string& errorMessage);

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

#endif //NOBLEENGINE_VULKANCONTEXT_H
