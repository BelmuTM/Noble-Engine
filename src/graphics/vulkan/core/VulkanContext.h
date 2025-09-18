#pragma once
#ifndef NOBLEENGINE_VULKANCONTEXT_H
#define NOBLEENGINE_VULKANCONTEXT_H

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"
#include "graphics/vulkan/pipeline/VulkanGraphicsPipeline.h"

#include <functional>

class VulkanContext {
public:
    VulkanContext() = default;
    ~VulkanContext();

    bool create(const Platform::Window& window, std::string& errorMessage);
    void destroy();

    VulkanInstance&  getInstance()  { return instance; }
    VulkanDevice&    getDevice()    { return device; }
    VulkanSurface&   getSurface()   { return surface; }
    VulkanSwapchain& getSwapchain() { return swapchain; }

private:
    VulkanInstance  instance;
    VulkanDevice    device;
    VulkanSurface   surface;
    VulkanSwapchain swapchain;

    std::vector<std::function<void()>> entityDeletionQueue;

    template<typename Resource, typename... Args>
    bool createVulkanEntity(Resource& res, std::string& errorMessage, Args&&... args) {
        if (!res.create(std::forward<Args>(args)..., errorMessage)) {
            return false;
        }
        entityDeletionQueue.push_back([&res] { res.destroy(); });
        return true;
    }
};

#endif //NOBLEENGINE_VULKANCONTEXT_H
