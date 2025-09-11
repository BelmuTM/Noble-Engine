#pragma once
#ifndef BAZARENGINE_VULKANCONTEXT_H
#define BAZARENGINE_VULKANCONTEXT_H

#include "graphics/GraphicsAPI.h"

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSurface.h"

#include <vulkan/vulkan.h>

class VulkanContext final : public GraphicsAPI {
public:
    VulkanContext() = default;
    ~VulkanContext() override;

    bool init(const WindowHandle& _window) override;
    void shutdown() override;
    void drawFrame() override;

private:
    VulkanInstance instance;
    VulkanDevice   device;
    VulkanSurface  surface;

    struct SwapchainSupportInfo {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;
    };

    static SwapchainSupportInfo querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR _surface);
    void                        createSwapchain();
};

#endif //BAZARENGINE_VULKANCONTEXT_H
