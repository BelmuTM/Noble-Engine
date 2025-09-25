#pragma once
#ifndef NOBLEENGINE_VULKANCONTEXT_H
#define NOBLEENGINE_VULKANCONTEXT_H

#include "graphics/vulkan/common/VulkanEntityOwner.h"

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"

class VulkanContext final : public VulkanEntityOwner<VulkanContext> {
public:
    VulkanContext() = default;
    ~VulkanContext();

    bool create(const Platform::Window& window, std::string& errorMessage);

    void destroy();

    VulkanInstance&  getInstance()  { return instance; }
    VulkanDevice&    getDevice()    { return device; }
    VulkanSurface&   getSurface()   { return surface; }
    VulkanSwapchain& getSwapchain() { return swapchain; }

    const VulkanInstance&  getInstance()  const { return instance; }
    const VulkanDevice&    getDevice()    const { return device; }
    const VulkanSurface&   getSurface()   const { return surface; }
    const VulkanSwapchain& getSwapchain() const { return swapchain; }

private:
    VulkanInstance  instance;
    VulkanSurface   surface;
    VulkanDevice    device;
    VulkanSwapchain swapchain;
};

#endif //NOBLEENGINE_VULKANCONTEXT_H
