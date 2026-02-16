#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

struct VulkanDescriptorBindingInfo {
    uint32_t             binding;
    vk::DescriptorType   type;
    vk::ShaderStageFlags stageFlags;
    uint32_t             count = 1;
    std::string          name  = "Undefined_DescriptorBinding";
};

using VulkanDescriptorScheme = std::vector<VulkanDescriptorBindingInfo>;

struct VulkanDescriptorInfo {
    vk::DescriptorType       type;
    vk::DescriptorImageInfo  imageInfo{};
    vk::DescriptorBufferInfo bufferInfo{};
    uint32_t                 binding;
};
