#pragma once

#include "VulkanShaderProgram.h"

#include <vector>

struct VulkanPipelineDescriptor {
    const VulkanShaderProgram* shaderProgram = nullptr;

    std::vector<vk::DescriptorSetLayout> descriptorLayouts{};
};
