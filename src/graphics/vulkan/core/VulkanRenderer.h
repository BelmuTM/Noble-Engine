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

    bool init(const Platform::Window& window) override;
    void shutdown() override;
    void drawFrame() override;

    VulkanContext& getContext() { return context; }

private:
    VulkanContext          context;
    VulkanGraphicsPipeline graphicsPipeline;

    vk::CommandPool   commandPool{};
    vk::CommandBuffer commandBuffer{};

    vk::Semaphore              imageAvailableSemaphore{};
    std::vector<vk::Semaphore> renderFinishedSemaphores{};
    vk::Fence                  drawFence{};

    bool createCommandPool(std::string& errorMessage);
    bool createCommandBuffer(std::string& errorMessage);

    bool createSyncObjects(std::string& errorMessage);

    void transitionImageLayout(
        uint32_t                imageIndex,
        vk::ImageLayout         oldLayout,
        vk::ImageLayout         newLayout,
        vk::AccessFlags2        srcAccessMask,
        vk::AccessFlags2        dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask
    );

    void recordCommandBuffer(uint32_t imageIndex);
};

#endif //NOBLEENGINE_VULKANRENDERER_H
