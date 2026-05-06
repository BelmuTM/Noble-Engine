#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include <optional>

namespace VulkanImageLayoutTransitions {
    struct LayoutTransition {
        vk::AccessFlags2 srcAccessMask;
        vk::AccessFlags2 dstAccessMask;

        vk::PipelineStageFlags2 srcStage;
        vk::PipelineStageFlags2 dstStage;
    };

    [[nodiscard]] std::optional<LayoutTransition> getLayoutTransition(
        vk::ImageLayout oldLayout, vk::ImageLayout newLayout
    );

    [[nodiscard]] Expected<void> transitionImageLayout(
        vk::CommandBuffer commandBuffer,
        vk::Image         image,
        vk::Format        format,
        vk::ImageLayout   oldLayout,
        vk::ImageLayout   newLayout,
        std::uint32_t     mipLevels = 1
    );
}
