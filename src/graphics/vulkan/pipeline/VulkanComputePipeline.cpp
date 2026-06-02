#include "VulkanComputePipeline.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

Expected<void> VulkanComputePipeline::create(const vk::Device& device, const VulkanRenderPass& pass) noexcept {
    _device = device;

    TRY(createPipelineLayout(device, pass.getPipelineDescriptor()));
    TRY(createComputePipeline(device, pass));

    return {};
}

Expected<void> VulkanComputePipeline::createComputePipeline(const vk::Device& device, const VulkanRenderPass& pass) {
    vk::ComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo
        // TODO: Fetch stage info by flag
        .setStage(pass.getPipelineDescriptor().shaderProgram->getStages().at(0))
        .setLayout(_pipelineLayout);

    VK_CREATE(_pipeline, device.createComputePipeline(nullptr, pipelineInfo));

    return {};
}
