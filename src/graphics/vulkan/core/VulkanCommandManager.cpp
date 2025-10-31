#include "VulkanCommandManager.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "core/debug/ErrorHandling.h"

bool VulkanCommandManager::create(
    const VulkanDevice& device, const uint32_t commandBufferCount, std::string& errorMessage
) noexcept {
    _device = &device;

    TRY(createCommandPool(errorMessage));
    TRY(createCommandBuffers(_commandBuffers, commandBufferCount, errorMessage));

    return true;
}

void VulkanCommandManager::destroy() noexcept {
    if (!_device) return;

    if (_commandPool) {
        _device->getLogicalDevice().destroyCommandPool(_commandPool);
        _commandPool = VK_NULL_HANDLE;
    }

    _device = nullptr;
}

bool VulkanCommandManager::createCommandPool(std::string& errorMessage) {
    vk::CommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(_device->getQueueFamilyIndices().graphicsFamily);

    const vk::Device& logicalDevice = _device->getLogicalDevice();

    VK_CREATE(logicalDevice.createCommandPool(commandPoolInfo), _commandPool, errorMessage);

    return true;
}

bool VulkanCommandManager::createCommandBuffers(
    std::vector<vk::CommandBuffer>& commandBuffers, const uint32_t commandBufferCount, std::string& errorMessage
) const {
    vk::CommandBufferAllocateInfo allocateInfo{};
    allocateInfo
        .setCommandPool(_commandPool)
        .setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandBufferCount(commandBufferCount);

    const vk::Device& logicalDevice = _device->getLogicalDevice();

    VK_CREATE(logicalDevice.allocateCommandBuffers(allocateInfo), commandBuffers, errorMessage);

    return true;
}

bool VulkanCommandManager::createCommandBuffer(vk::CommandBuffer& commandBuffer, std::string& errorMessage) const {
    std::vector<vk::CommandBuffer> commandBuffers;

    if (!createCommandBuffers(commandBuffers, 1, errorMessage)) return false;
    commandBuffer = std::move(commandBuffers.front());

    return true;
}

bool VulkanCommandManager::beginSingleTimeCommands(vk::CommandBuffer& commandBuffer, std::string& errorMessage) const {
    TRY(createCommandBuffer(commandBuffer, errorMessage));

    constexpr vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
    VK_TRY(commandBuffer.begin(beginInfo), errorMessage);

    return true;
}

bool VulkanCommandManager::endSingleTimeCommands(vk::CommandBuffer& commandBuffer, std::string& errorMessage) const {
    VK_TRY(commandBuffer.end(), errorMessage);

    vk::SubmitInfo submitInfo{};
    submitInfo.setCommandBuffers(commandBuffer);

    const vk::Queue& graphicsQueue = _device->getGraphicsQueue();

    VK_TRY(graphicsQueue.submit(submitInfo), errorMessage);
    VK_TRY(graphicsQueue.waitIdle(), errorMessage);

    return true;
}
