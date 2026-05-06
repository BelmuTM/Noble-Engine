#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/core/VulkanDevice.h"

class VulkanCommandManager {
public:
    VulkanCommandManager()  = default;
    ~VulkanCommandManager() = default;

    VulkanCommandManager(const VulkanCommandManager&)            = delete;
    VulkanCommandManager& operator=(const VulkanCommandManager&) = delete;

    VulkanCommandManager(VulkanCommandManager&&)            = delete;
    VulkanCommandManager& operator=(VulkanCommandManager&&) = delete;

    [[nodiscard]] Expected<void> create(const VulkanDevice& device, std::uint32_t commandBufferCount) noexcept;

    void destroy() noexcept;

    [[nodiscard]] Expected<void> createCommandBuffers(
        std::vector<vk::CommandBuffer>& commandBuffers, std::uint32_t commandBufferCount
    ) const;

    [[nodiscard]] Expected<void> createCommandBuffer(vk::CommandBuffer& commandBuffer) const;

    [[nodiscard]] Expected<void> beginSingleTimeCommands(vk::CommandBuffer& commandBuffer) const;

    [[nodiscard]] Expected<void> endSingleTimeCommands(vk::CommandBuffer& commandBuffer) const;

    [[nodiscard]]       std::vector<vk::CommandBuffer>& getCommandBuffers()       noexcept { return _commandBuffers; }
    [[nodiscard]] const std::vector<vk::CommandBuffer>& getCommandBuffers() const noexcept { return _commandBuffers; }

private:
    Expected<void> createCommandPool();

    const VulkanDevice* _device = nullptr;

    vk::CommandPool                _commandPool{};
    std::vector<vk::CommandBuffer> _commandBuffers{};
};
