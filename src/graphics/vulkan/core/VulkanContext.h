#pragma once
#ifndef NOBLEENGINE_VULKANCONTEXT_H
#define NOBLEENGINE_VULKANCONTEXT_H

#include "graphics/GraphicsAPI.h"

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"

#include "../pipeline/VulkanGraphicsPipeline.h"

#include "core/Engine.h"

#include <functional>

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
    VulkanGraphicsPipeline pipeline;

    std::vector<std::function<void()>> entityDeletionQueue;

    template<typename Resource, typename... Args>
    void createVulkanEntity(Resource& res, std::string& errorMessage, Args&&... args) {
        if (!res.create(std::forward<Args>(args)..., errorMessage)) {
            Engine::fatalExit(errorMessage);
        }
        entityDeletionQueue.push_back([&res] { res.destroy(); });
    }
};

#endif //NOBLEENGINE_VULKANCONTEXT_H
