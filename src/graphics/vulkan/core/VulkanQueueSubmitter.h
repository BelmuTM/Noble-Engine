#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

class VulkanQueueSubmitter {
public:
    [[nodiscard]] static Expected<void> submit(
        vk::Queue                                queue,
        vk::CommandBuffer                        commandBuffer,
        std::span<const vk::SemaphoreSubmitInfo> waits,
        std::span<const vk::SemaphoreSubmitInfo> signals,
        vk::Fence                                fence
    );
};
