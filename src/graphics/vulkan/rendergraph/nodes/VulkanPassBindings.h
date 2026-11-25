#pragma once

class VulkanPipeline;
class VulkanDescriptorSets;

struct VulkanPassBindings {
    std::span<const VulkanPipeline* const> pipelines{};

    const VulkanDescriptorSets* sets = nullptr;
};
