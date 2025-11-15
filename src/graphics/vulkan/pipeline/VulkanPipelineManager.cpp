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
        graphicsPipeline->destroy();
    }

    _graphicsPipelines.clear();

    _device    = VK_NULL_HANDLE;
    _swapchain = nullptr;
}

VulkanGraphicsPipeline* VulkanPipelineManager::allocatePipeline() {
    _graphicsPipelines.emplace_back(std::make_unique<VulkanGraphicsPipeline>());
    return _graphicsPipelines.back().get();
}

bool VulkanPipelineManager::createGraphicsPipeline(
    VulkanGraphicsPipeline*                     graphicsPipeline,
    const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
    const VulkanShaderProgram&                  shaderProgram,
    std::string&                                errorMessage
) {
    TRY(graphicsPipeline->create(_device, *_swapchain, descriptorSetLayouts, {}, shaderProgram, errorMessage));

    _graphicsPipelines.emplace_back(graphicsPipeline);

    return true;
}

bool VulkanPipelineManager::createGraphicsPipeline(
    VulkanGraphicsPipeline*                     graphicsPipeline,
    const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
    const uint32_t                              pushConstantRangeSize,
    const VulkanShaderProgram&                  shaderProgram,
    std::string&                                errorMessage
) {
    // TO-DO: use shader reflection for push constant size
    vk::PushConstantRange pushConstantRange{};
    pushConstantRange
        .setStageFlags(shaderProgram.getStageFlags())
        .setOffset(0)
        .setSize(pushConstantRangeSize);

    TRY(graphicsPipeline->create(
        _device, *_swapchain, descriptorSetLayouts, {pushConstantRange}, shaderProgram, errorMessage
    ));

    _graphicsPipelines.emplace_back(graphicsPipeline);

    return true;
}
