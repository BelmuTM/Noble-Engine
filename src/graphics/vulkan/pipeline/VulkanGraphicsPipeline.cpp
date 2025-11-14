#include "VulkanGraphicsPipeline.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/resources/meshes/VulkanVertex.h"

#include "core/debug/ErrorHandling.h"

bool VulkanGraphicsPipeline::create(
    const vk::Device&                           device,
    const VulkanSwapchain&                      swapchain,
    const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
    const std::vector<vk::PushConstantRange>&   pushConstantRanges,
    const VulkanShaderProgram&                  shaderProgram,
    std::string&                                errorMessage
) noexcept {
    _device = device;

    TRY(createPipelineLayout(device, descriptorSetLayouts, pushConstantRanges, errorMessage));
    TRY(createPipeline(device, swapchain, shaderProgram, errorMessage));

    _stageFlags = shaderProgram.getStageFlags();

    return true;
}

void VulkanGraphicsPipeline::destroy() noexcept {
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

bool VulkanGraphicsPipeline::createPipelineLayout(
    const vk::Device&                           device,
    const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
    const std::vector<vk::PushConstantRange>&   pushConstantRanges,
    std::string&                                errorMessage
) {
    vk::PipelineLayoutCreateInfo layoutInfo{};
    layoutInfo
        .setSetLayouts(descriptorSetLayouts)
        .setPushConstantRanges(pushConstantRanges);

    VK_CREATE(device.createPipelineLayout(layoutInfo), _pipelineLayout, errorMessage);

    return true;
}

bool VulkanGraphicsPipeline::createPipeline(
    const vk::Device&          device,
    const VulkanSwapchain&     swapchain,
    const VulkanShaderProgram& shaderProgram,
    std::string&               errorMessage
) {
    const vk::Format& colorFormat = swapchain.getFormat();

    vk::PipelineRenderingCreateInfo renderingInfo{};
    renderingInfo
        .setColorAttachmentFormats(colorFormat)
        .setDepthAttachmentFormat(vk::Format::eD32Sfloat);

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

    const auto& bindingDescription    = VulkanVertex::getBindingDescription();
    const auto& attributeDescriptions = VulkanVertex::getAttributeDescriptions();

    if (!shaderProgram.isFullscreen()) {
        vertexInputInfo
            .setVertexBindingDescriptions(bindingDescription)
            .setVertexAttributeDescriptions(attributeDescriptions);
    }

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo
        .setPNext(&renderingInfo)
        .setStages(shaderProgram.getStages())
        .setPVertexInputState(&vertexInputInfo)
        .setPInputAssemblyState(&inputAssemblyInfo)
        .setPViewportState(&viewportInfo)
        .setPRasterizationState(&rasterizationInfo)
        .setPMultisampleState(&multisamplingInfo)
        .setPDepthStencilState(&depthStencilInfo)
        .setPColorBlendState(&colorBlendInfo)
        .setPDynamicState(&dynamicStateInfo)
        .setLayout(_pipelineLayout)
        .setRenderPass(nullptr);

    VK_CREATE(device.createGraphicsPipeline(nullptr, pipelineInfo), _pipeline, errorMessage);

    return true;
}
