#include "VulkanQueueSubmitter.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

Expected<void> VulkanQueueSubmitter::submit(
    const vk::Queue                          queue,
    const vk::CommandBuffer                  commandBuffer,
    std::span<const vk::SemaphoreSubmitInfo> waits,
    std::span<const vk::SemaphoreSubmitInfo> signals,
    const vk::Fence                          fence
) {
    vk::CommandBufferSubmitInfo commandBufferInfo{};
    commandBufferInfo.setCommandBuffer(commandBuffer);

    vk::SubmitInfo2 submitInfo{};
    submitInfo
        .setWaitSemaphoreInfos(waits)
        .setCommandBufferInfos(commandBufferInfo)
        .setSignalSemaphoreInfos(signals);

    VK_TRY(queue.submit2(submitInfo, fence));

    return {};
}
