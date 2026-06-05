#include "VulkanComputePipelineManager.h"

Expected<void> VulkanComputePipelineManager::create(const vk::Device& device) noexcept {
    _device = device;

    return {};
}

void VulkanComputePipelineManager::destroy() noexcept {
    for (const auto& computePipeline : _computePipelines) {
        computePipeline->destroy();
    }

    _device = VK_NULL_HANDLE;
}

Expected<VulkanComputePipeline*> VulkanComputePipelineManager::createComputePipeline(
    const VulkanComputePipelineDescriptor& descriptor
) {
    _computePipelines.emplace_back(std::make_unique<VulkanComputePipeline>());

    TRY(_computePipelines.back()->create(_device, descriptor));

    return Expected(_computePipelines.back().get());
}
