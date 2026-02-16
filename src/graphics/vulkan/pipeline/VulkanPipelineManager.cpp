#include "VulkanPipelineManager.h"

#include "core/debug/ErrorHandling.h"

bool VulkanPipelineManager::create(const vk::Device& device, std::string& errorMessage) noexcept {
    _device = device;

    return true;
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

bool VulkanPipelineManager::createGraphicsPipeline(
    VulkanGraphicsPipeline* graphicsPipeline,
    const VulkanRenderPass& pass,
    std::string&            errorMessage
) const {
    TRY(graphicsPipeline->create(_device, pass, errorMessage));

    return true;
}
