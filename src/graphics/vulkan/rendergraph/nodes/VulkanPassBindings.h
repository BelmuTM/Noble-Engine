#pragma once

#include <string>
#include <vector>

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorManager.h"

class VulkanPipeline;
class VulkanDescriptorSets;

struct VulkanPassBindings {
    std::span<const VulkanPipeline* const> pipelines{};

    const VulkanDescriptorSets* sets = nullptr;
};

struct VulkanPassDescriptorBinding {
    VulkanDescriptorManager manager{};
    VulkanDescriptorSets*   sets = nullptr;

    std::vector<std::string> sampledResourceNames{};
};
