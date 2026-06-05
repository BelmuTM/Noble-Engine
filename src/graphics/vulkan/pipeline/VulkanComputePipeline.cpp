#include "VulkanComputePipeline.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

Expected<void> VulkanComputePipeline::create(
    const vk::Device& device, const VulkanComputePipelineDescriptor& descriptor
) noexcept {
    _device = device;

    TRY(createPipelineLayout(device, descriptor.layout));
    TRY(createComputePipeline(device, descriptor));

    return {};
}

Expected<void> VulkanComputePipeline::createComputePipeline(
    const vk::Device& device, const VulkanComputePipelineDescriptor& descriptor
) {
    vk::ComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo
        // TODO: Fetch stage info by flag
        .setStage(descriptor.shaderStages[0])
        .setLayout(_pipelineLayout);

    VK_CREATE(_pipeline, device.createComputePipeline(nullptr, pipelineInfo));

    return {};
}
