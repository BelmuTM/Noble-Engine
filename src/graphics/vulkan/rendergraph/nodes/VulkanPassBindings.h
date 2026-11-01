#pragma once

class VulkanPipeline;
class VulkanPipelineLayoutDescriptor;
class VulkanDescriptorSets;

struct VulkanPassBindings {
    const VulkanPipeline*                 pipeline         = nullptr;
    const VulkanPipelineLayoutDescriptor* layoutDescriptor = nullptr;

    const VulkanDescriptorSets* sets = nullptr;
};
