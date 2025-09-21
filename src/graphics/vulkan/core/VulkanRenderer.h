#pragma once
#ifndef NOBLEENGINE_VULKANRENDERER_H
#define NOBLEENGINE_VULKANRENDERER_H

#include "graphics/GraphicsAPI.h"
#include "graphics/vulkan/common/VulkanEntityOwner.h"

#include "core/platform/Platform.h"

#include "VulkanCommandManager.h"
#include "VulkanContext.h"
#include "VulkanSyncObjects.h"
#include "graphics/vulkan/pipeline/VulkanGraphicsPipeline.h"

class VulkanRenderer final : public GraphicsAPI, public VulkanEntityOwner<VulkanRenderer> {
public:
    VulkanRenderer() = default;
    ~VulkanRenderer() override;

    bool init(Platform::Window& window) override;
    void shutdown() override;
    void drawFrame() override;

    VulkanContext& getContext() { return context; }

private:
    Platform::Window* _window = nullptr;

    VulkanContext          context;
    VulkanGraphicsPipeline graphicsPipeline;
    VulkanSyncObjects      syncObjects;
    VulkanCommandManager   commandManager;

    unsigned int currentFrame = 0;

    bool handleFramebufferResize(std::string& errorMessage);

    std::optional<uint32_t> acquireNextImage(std::string& errorMessage);

    void waitForImageFence(uint32_t imageIndex);

    void recordCurrentCommandBuffer(uint32_t imageIndex);
    bool submitCommandBuffer(uint32_t imageIndex, std::string& errorMessage);

    bool recreateSwapchain(std::string& errorMessage);
};

#endif //NOBLEENGINE_VULKANRENDERER_H
