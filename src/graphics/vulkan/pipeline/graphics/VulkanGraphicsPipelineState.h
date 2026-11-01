#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

struct VulkanGraphicsPipelineState {
    vk::PrimitiveTopology topology        = vk::PrimitiveTopology::eTriangleList;
    vk::PolygonMode       polygonMode     = vk::PolygonMode::eFill;
    vk::CullModeFlags     cullMode        = vk::CullModeFlagBits::eBack;
    float                 lineWidth       = 1.0f;
    bool                  meshVertexInput = true;
};
