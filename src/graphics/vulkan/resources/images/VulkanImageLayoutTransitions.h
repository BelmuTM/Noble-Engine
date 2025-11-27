#pragma once
#ifndef NOBLEENGINE_VULKANIMAGELAYOUTTRANSITIONS_H
#define NOBLEENGINE_VULKANIMAGELAYOUTTRANSITIONS_H

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

    [[nodiscard]] bool transitionImageLayout(
        vk::CommandBuffer commandBuffer,
        std::string&      errorMessage,
        vk::Image         image,
        vk::Format        format,
        vk::ImageLayout   oldLayout,
        vk::ImageLayout   newLayout,
        uint32_t          mipLevels = 1
    );
}

#endif // NOBLEENGINE_VULKANIMAGELAYOUTTRANSITIONS_H
