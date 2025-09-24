#include "VulkanCommandManager.h"

#include "core/debug/Logger.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

bool VulkanCommandManager::create(
    const VulkanDevice& device, const uint32_t commandBufferCount, std::string& errorMessage
) noexcept {
    _device = &device;

    if (!createCommandPool(errorMessage)) return false;
    if (!createCommandBuffers(_commandBuffers, commandBufferCount, errorMessage)) return false;
    return true;
}

void VulkanCommandManager::destroy() noexcept {
    if (!_device) return;

    if (commandPool) {
        _device->getLogicalDevice().destroyCommandPool(commandPool);
        commandPool = VK_NULL_HANDLE;
    }

    _device = nullptr;
}

bool VulkanCommandManager::createCommandPool(std::string& errorMessage) {
    vk::CommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(_device->getQueueFamilyIndices().graphicsFamily);

    const vk::Device& logicalDevice = _device->getLogicalDevice();

    VK_CREATE(logicalDevice.createCommandPool(commandPoolInfo), commandPool, errorMessage);
    return true;
}

bool VulkanCommandManager::createCommandBuffers(
    std::vector<vk::CommandBuffer>& commandBuffers, const uint32_t commandBufferCount, std::string& errorMessage
) const {
    vk::CommandBufferAllocateInfo allocateInfo{};
    allocateInfo
        .setCommandPool(commandPool)
        .setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandBufferCount(commandBufferCount);

    const vk::Device& logicalDevice = _device->getLogicalDevice();

    // TO-DO: write memory allocator helper to prevent simultaneous allocations (mighth hit device allocation limit)
    VK_CREATE(logicalDevice.allocateCommandBuffers(allocateInfo), commandBuffers, errorMessage);
    return true;
}

bool VulkanCommandManager::createCommandBuffer(vk::CommandBuffer& commandBuffer, std::string& errorMessage) const {
    std::vector<vk::CommandBuffer> commandBuffers;
    if (!createCommandBuffers(commandBuffers, 1, errorMessage)) return false;
    commandBuffer = commandBuffers.front();
    return true;
}
