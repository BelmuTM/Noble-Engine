#pragma once

class VulkanPipeline;
class VulkanDescriptorSets;

struct VulkanPassBindings {
    const VulkanPipeline*       pipeline = nullptr;
    const VulkanDescriptorSets* sets     = nullptr;
};
