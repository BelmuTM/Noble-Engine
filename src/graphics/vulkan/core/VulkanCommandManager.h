#pragma once
#ifndef NOBLEENGINE_VULKANCOMMANDMANAGER_H
#define NOBLEENGINE_VULKANCOMMANDMANAGER_H

#include "VulkanSwapchain.h"
#include "graphics/vulkan/common/VulkanHeader.h"
#include "graphics/vulkan/pipeline/VulkanGraphicsPipeline.h"

class VulkanCommandManager {
public:
    VulkanCommandManager()  = default;
    ~VulkanCommandManager() = default;

    VulkanCommandManager(const VulkanCommandManager&)            = delete;
    VulkanCommandManager& operator=(const VulkanCommandManager&) = delete;
    VulkanCommandManager(VulkanCommandManager&&)                 = delete;
    VulkanCommandManager& operator=(VulkanCommandManager&&)      = delete;

    [[nodiscard]] bool create(
        const VulkanDevice& device, const VulkanSwapchain& swapchain, uint32_t commandBufferCount,
        std::string& errorMessage
    ) noexcept;
    void destroy() noexcept;

    [[nodiscard]] std::vector<vk::CommandBuffer>& getCommandBuffers() { return commandBuffers; }

    void recordCommandBuffer(
        vk::CommandBuffer commandBuffer, uint32_t imageIndex, const VulkanGraphicsPipeline& pipeline
    ) const;

private:
    const VulkanDevice*    _device    = nullptr;
    const VulkanSwapchain* _swapchain = nullptr;

    vk::CommandPool                commandPool{};
    std::vector<vk::CommandBuffer> commandBuffers{};

    bool createCommandPool(std::string& errorMessage);
    bool createCommandBuffers(uint32_t commandBufferCount, std::string& errorMessage);

    void transitionImageLayout(
        vk::CommandBuffer       commandBuffer,
        uint32_t                imageIndex,
        vk::ImageLayout         oldLayout,
        vk::ImageLayout         newLayout,
        vk::AccessFlags2        srcAccessMask,
        vk::AccessFlags2        dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask
    ) const;
};

#endif //NOBLEENGINE_VULKANCOMMANDMANAGER_H
