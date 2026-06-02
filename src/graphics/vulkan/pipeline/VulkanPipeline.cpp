#include "VulkanPipeline.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include <ranges>

void VulkanPipeline::destroy() noexcept {
    if (!_device) return;

    if (_pipeline) {
        _device.destroyPipeline(_pipeline);
        _pipeline = VK_NULL_HANDLE;
    }

    if (_pipelineLayout) {
        _device.destroyPipelineLayout(_pipelineLayout);
        _pipelineLayout = VK_NULL_HANDLE;
    }

    _device = VK_NULL_HANDLE;
}

Expected<void> VulkanPipeline::createPipelineLayout(
    const vk::Device& device, const VulkanPipelineDescriptor& descriptor
) {
    std::vector<vk::PushConstantRange> _pushConstantRanges{};

    for (const auto& [stageFlags, offset, size] : descriptor.shaderProgram->getPushConstants() | std::views::values) {
        _pushConstantRanges.emplace_back(stageFlags, offset, size);
    }

    vk::PipelineLayoutCreateInfo layoutInfo{};
    layoutInfo
        .setSetLayouts(descriptor.descriptorLayouts)
        .setPushConstantRanges(_pushConstantRanges);

    VK_CREATE(_pipelineLayout, device.createPipelineLayout(layoutInfo));

    return {};
}
