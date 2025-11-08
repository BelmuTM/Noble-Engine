#include "VulkanPipelineManager.h"

#include "core/debug/ErrorHandling.h"

bool VulkanPipelineManager::create(
    const vk::Device& device, const VulkanSwapchain& swapchain, std::string& errorMessage
) noexcept {
    _device    = device;
    _swapchain = &swapchain;
    return true;
}

void VulkanPipelineManager::destroy() noexcept {
    for (const auto& graphicsPipeline : _graphicsPipelines) {
        graphicsPipeline->destroy(_device);
    }

    _graphicsPipelines.clear();

    _device    = VK_NULL_HANDLE;
    _swapchain = nullptr;
}

bool VulkanPipelineManager::createGraphicsPipeline(
    VulkanGraphicsPipeline&                     graphicsPipeline,
    const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
    const VulkanShaderProgram&                  shaderProgram,
    std::string&                                errorMessage
) {
    TRY(graphicsPipeline.create(_device, *_swapchain, descriptorSetLayouts, shaderProgram, errorMessage));

    _graphicsPipelines.push_back(&graphicsPipeline);

    return true;
}
