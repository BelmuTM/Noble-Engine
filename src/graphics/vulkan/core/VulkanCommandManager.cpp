#include "VulkanCommandManager.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

Expected<void> VulkanCommandManager::create(
    const VulkanDevice& device, const std::uint32_t commandBufferCount
) noexcept {
    _device = &device;

    TRY(createCommandPool());
    TRY(createCommandBuffers(_commandBuffers, commandBufferCount));

    return {};
}

void VulkanCommandManager::destroy() noexcept {
    if (!_device) return;

    if (_commandPool) {
        _device->getLogicalDevice().destroyCommandPool(_commandPool);
        _commandPool = VK_NULL_HANDLE;
    }

    _device = nullptr;
}

Expected<void> VulkanCommandManager::createCommandPool() {
    vk::CommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(_device->getQueueFamilyIndices().graphicsFamily);

    const vk::Device& logicalDevice = _device->getLogicalDevice();

    VK_CREATE(_commandPool, logicalDevice.createCommandPool(commandPoolInfo));

    return {};
}

Expected<void> VulkanCommandManager::createCommandBuffers(
    std::vector<vk::CommandBuffer>& commandBuffers, const std::uint32_t commandBufferCount
) const {
    vk::CommandBufferAllocateInfo allocateInfo{};
    allocateInfo
        .setCommandPool(_commandPool)
        .setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandBufferCount(commandBufferCount);

    const vk::Device& logicalDevice = _device->getLogicalDevice();

    VK_CREATE(commandBuffers, logicalDevice.allocateCommandBuffers(allocateInfo));

    return {};
}

Expected<void> VulkanCommandManager::createCommandBuffer(vk::CommandBuffer& commandBuffer) const {
    std::vector<vk::CommandBuffer> commandBuffers;

    TRY(createCommandBuffers(commandBuffers, 1));
    commandBuffer = commandBuffers.front();

    return {};
}

Expected<void> VulkanCommandManager::beginSingleTimeCommands(vk::CommandBuffer& commandBuffer) const {
    TRY(createCommandBuffer(commandBuffer));

    constexpr vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
    VK_TRY(commandBuffer.begin(beginInfo));

    return {};
}

Expected<void> VulkanCommandManager::endSingleTimeCommands(vk::CommandBuffer& commandBuffer) const {
    VK_TRY(commandBuffer.end());

    vk::SubmitInfo submitInfo{};
    submitInfo.setCommandBuffers(commandBuffer);

    const vk::Queue& graphicsQueue = _device->getGraphicsQueue();

    VK_TRY(graphicsQueue.submit(submitInfo));
    VK_TRY(graphicsQueue.waitIdle());

    return {};
}
