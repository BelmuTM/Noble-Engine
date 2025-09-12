#pragma once
#ifndef BAZARENGINE_VULKANCONTEXT_H
#define BAZARENGINE_VULKANCONTEXT_H

#include "graphics/GraphicsAPI.h"

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"

class VulkanContext final : public GraphicsAPI {
public:
    VulkanContext() = default;
    ~VulkanContext() override;

    bool init(const Platform::Window& window) override;
    void shutdown() override;
    void drawFrame() override;

private:
    VulkanInstance  instance;
    VulkanDevice    device;
    VulkanSurface   surface;
    VulkanSwapchain swapchain;
};

#endif //BAZARENGINE_VULKANCONTEXT_H
