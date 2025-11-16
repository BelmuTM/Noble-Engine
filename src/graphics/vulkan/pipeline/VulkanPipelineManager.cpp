#include "VulkanPipelineManager.h"

#include "core/debug/ErrorHandling.h"

bool VulkanPipelineManager::create(
    const vk::Device& device, std::string& errorMessage
) noexcept {
    _device = device;
    return true;
}

void VulkanPipelineManager::destroy() noexcept {
    for (const auto& graphicsPipeline : _graphicsPipelines) {
        graphicsPipeline->destroy();
    }

    _graphicsPipelines.clear();

    _device = VK_NULL_HANDLE;
}

VulkanGraphicsPipeline* VulkanPipelineManager::allocatePipeline() {
    _graphicsPipelines.emplace_back(std::make_unique<VulkanGraphicsPipeline>());
    return _graphicsPipelines.back().get();
}

bool VulkanPipelineManager::createGraphicsPipeline(
    VulkanGraphicsPipeline*                        graphicsPipeline,
    const VulkanPipelineDescriptor&                descriptor,
    const std::vector<VulkanRenderPassAttachment>& colorAttachments,
    std::string&                                   errorMessage
) {
    TRY(graphicsPipeline->create(_device, descriptor, colorAttachments, errorMessage));

    _graphicsPipelines.emplace_back(graphicsPipeline);

    return true;
}
