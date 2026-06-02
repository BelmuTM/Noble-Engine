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

Expected<VulkanComputePipeline*> VulkanComputePipelineManager::createComputePipeline(const VulkanRenderPass& pass) {
    _computePipelines.emplace_back(std::make_unique<VulkanComputePipeline>());

    TRY(_computePipelines.back()->create(_device, pass));

    return Expected(_computePipelines.back().get());
}
