#pragma once

#include "graphics/vulkan/rendergraph/resources/VulkanGraphResource.h"
#include "graphics/vulkan/resources/descriptors/VulkanDescriptorManager.h"

#include <vector>

class VulkanPipeline;
class VulkanDescriptorSets;

struct VulkanPassBindings {
    std::span<const VulkanPipeline* const> pipelines{};

    const VulkanDescriptorSets* sets = nullptr;
};

struct VulkanPassDescriptorBinding {
    VulkanDescriptorManager manager{};
    VulkanDescriptorSets*   sets = nullptr;

    std::vector<VulkanGraphResourceId> sampledResources{};
};
