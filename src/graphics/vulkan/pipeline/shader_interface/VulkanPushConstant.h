#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include <unordered_map>

struct VulkanPushConstantRange {
    vk::ShaderStageFlags stageFlags;
    std::uint32_t        offset = 0;
    std::uint32_t        size   = 0;
};

struct VulkanPushConstant {
    const void*             data = nullptr;
    VulkanPushConstantRange range{};
};

using VulkanPushConstantsMap = std::unordered_map<std::string, VulkanPushConstantRange>;
