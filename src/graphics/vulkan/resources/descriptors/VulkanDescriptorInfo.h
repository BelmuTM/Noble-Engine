#pragma once
#ifndef NOBLEENGINE_VULKANDESCRIPTORINFO_H
#define NOBLEENGINE_VULKANDESCRIPTORINFO_H

#include "graphics/vulkan/common/VulkanHeader.h"

struct DescriptorInfo {
    vk::DescriptorType       type;
    vk::DescriptorImageInfo  imageInfo{};
    vk::DescriptorBufferInfo bufferInfo{};
    uint32_t                 binding;
};

#endif // NOBLEENGINE_VULKANDESCRIPTORINFO_H
