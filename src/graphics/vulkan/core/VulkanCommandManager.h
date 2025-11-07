#pragma once
#ifndef NOBLEENGINE_VULKANCOMMANDMANAGER_H
#define NOBLEENGINE_VULKANCOMMANDMANAGER_H

#include "graphics/vulkan/common/VulkanHeader.h"
#include "VulkanDevice.h"

class VulkanCommandManager {
public:
    VulkanCommandManager()  = default;
    ~VulkanCommandManager() = default;

    VulkanCommandManager(const VulkanCommandManager&)            = delete;
    VulkanCommandManager& operator=(const VulkanCommandManager&) = delete;

    VulkanCommandManager(VulkanCommandManager&&)            = delete;
    VulkanCommandManager& operator=(VulkanCommandManager&&) = delete;

    [[nodiscard]] bool create(
        const VulkanDevice& device, uint32_t commandBufferCount, std::string& errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]]       std::vector<vk::CommandBuffer>& getCommandBuffers()       noexcept { return _commandBuffers; }
    [[nodiscard]] const std::vector<vk::CommandBuffer>& getCommandBuffers() const noexcept { return _commandBuffers; }

    [[nodiscard]] bool createCommandBuffers(
        std::vector<vk::CommandBuffer>& commandBuffers, uint32_t commandBufferCount, std::string& errorMessage
    ) const;

    [[nodiscard]] bool createCommandBuffer(vk::CommandBuffer& commandBuffer, std::string& errorMessage) const;

    [[nodiscard]] bool beginSingleTimeCommands(vk::CommandBuffer& commandBuffer, std::string& errorMessage) const;

    [[nodiscard]] bool endSingleTimeCommands(vk::CommandBuffer& commandBuffer, std::string& errorMessage) const;

private:
    const VulkanDevice* _device = nullptr;

    vk::CommandPool                _commandPool{};
    std::vector<vk::CommandBuffer> _commandBuffers{};

    bool createCommandPool(std::string& errorMessage);
};

#endif //NOBLEENGINE_VULKANCOMMANDMANAGER_H
