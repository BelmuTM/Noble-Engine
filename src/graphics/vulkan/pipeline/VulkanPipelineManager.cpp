#include "VulkanPipelineManager.h"

Expected<void> VulkanPipelineManager::create(const vk::Device& device) noexcept {
    _device = device;

    return {};
}

void VulkanPipelineManager::destroy() noexcept {
    for (const auto& graphicsPipeline : _graphicsPipelines) {
        graphicsPipeline->destroy();
    }

    _device = VK_NULL_HANDLE;
}

VulkanGraphicsPipeline* VulkanPipelineManager::allocatePipeline() {
    _graphicsPipelines.emplace_back(std::make_unique<VulkanGraphicsPipeline>());
    return _graphicsPipelines.back().get();
}

Expected<void> VulkanPipelineManager::createGraphicsPipeline(
    VulkanGraphicsPipeline* graphicsPipeline, const VulkanRenderPass& pass
) const {
    TRY(graphicsPipeline->create(_device, pass));

    return {};
}
