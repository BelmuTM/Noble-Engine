#include "VulkanGraphicsPipelineManager.h"

Expected<void> VulkanGraphicsPipelineManager::create(const vk::Device& device) noexcept {
    _device = device;

    return {};
}

void VulkanGraphicsPipelineManager::destroy() noexcept {
    for (const auto& graphicsPipeline : _graphicsPipelines) {
        graphicsPipeline->destroy();
    }

    _device = VK_NULL_HANDLE;
}

Expected<VulkanGraphicsPipeline*> VulkanGraphicsPipelineManager::createGraphicsPipeline(
    const VulkanGraphicsPipelineDescriptor& descriptor
) {
    _graphicsPipelines.emplace_back(std::make_unique<VulkanGraphicsPipeline>());

    TRY(_graphicsPipelines.back()->create(_device, descriptor));

    return Expected(_graphicsPipelines.back().get());
}
