#pragma once
#ifndef NOBLEENGINE_VULKANRENDERER_H
#define NOBLEENGINE_VULKANRENDERER_H

#include "graphics/GraphicsAPI.h"
#include "VulkanContext.h"

#include "graphics/vulkan/common/VulkanHeader.h"

class VulkanRenderer final : public GraphicsAPI {
public:
    VulkanRenderer() = default;
    ~VulkanRenderer() override;

    bool init(Platform::Window& window) override;
    void shutdown() override;
    void drawFrame() override;

    VulkanContext& getContext() { return context; }

private:
    Platform::Window* _window;

    VulkanContext          context;
    VulkanGraphicsPipeline graphicsPipeline;

    unsigned int currentFrame = 0;

    vk::CommandPool                commandPool{};
    std::vector<vk::CommandBuffer> commandBuffers{};

    std::vector<vk::Semaphore> imageAvailableSemaphores{};
    std::vector<vk::Semaphore> renderFinishedSemaphores{};
    std::vector<vk::Fence>     inFlightFences{};

    std::vector<vk::Fence>     oldFences{};
    std::vector<vk::Semaphore> oldImageAvailable{};
    std::vector<vk::Semaphore> oldRenderFinished{};

    std::vector<vk::Fence> imagesInFlight{};

    bool recreateSwapchain(std::string& errorMessage);

    bool createCommandPool(std::string& errorMessage);
    bool createCommandBuffer(std::string& errorMessage);

    bool createSyncObjects(std::string& errorMessage);
    void destroySyncObjects();
    void cleanupOldSyncObjects();

    void transitionImageLayout(
        vk::CommandBuffer       commandBuffer,
        uint32_t                imageIndex,
        vk::ImageLayout         oldLayout,
        vk::ImageLayout         newLayout,
        vk::AccessFlags2        srcAccessMask,
        vk::AccessFlags2        dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask
    );

    void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
};

#endif //NOBLEENGINE_VULKANRENDERER_H
