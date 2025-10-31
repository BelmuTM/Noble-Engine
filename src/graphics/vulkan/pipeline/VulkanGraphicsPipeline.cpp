#include "VulkanGraphicsPipeline.h"

#include "graphics/vulkan/common/VulkanDebugger.h"
#include "graphics/vulkan/resources/mesh/VulkanMesh.h"

bool VulkanGraphicsPipeline::create(
    const vk::Device&             device,
    const VulkanSwapchain&        swapchain,
    const vk::DescriptorSetLayout descriptorSetLayout,
    const VulkanShaderProgram&    shaderProgram,
    std::string&                  errorMessage
) noexcept {
    _device    = device;
    _swapchain = &swapchain;

    if (!createPipelineLayout(descriptorSetLayout, errorMessage)) return false;
    if (!createPipeline(shaderProgram, errorMessage))             return false;

    return true;
}

void VulkanGraphicsPipeline::destroy() noexcept {
    if (pipeline) {
        _device.destroyPipeline(pipeline);
        pipeline = VK_NULL_HANDLE;
    }

    if (pipelineLayout) {
        _device.destroyPipelineLayout(pipelineLayout);
        pipelineLayout = VK_NULL_HANDLE;
    }

    _swapchain = nullptr;
}

bool VulkanGraphicsPipeline::createPipelineLayout(
    vk::DescriptorSetLayout descriptorSetLayout, std::string& errorMessage) {
    vk::PipelineLayoutCreateInfo layoutInfo{};
    layoutInfo
        .setSetLayouts(descriptorSetLayout)
        .setPushConstantRangeCount(0);

    VK_CREATE(_device.createPipelineLayout(layoutInfo), pipelineLayout, errorMessage);

    return true;
}

bool VulkanGraphicsPipeline::createPipeline(const VulkanShaderProgram& shaderProgram, std::string& errorMessage) {
    const vk::Format& colorFormat = _swapchain->getFormat();

    vk::PipelineRenderingCreateInfo renderingInfo{};
    renderingInfo
        .setColorAttachmentFormats(colorFormat)
        .setDepthAttachmentFormat(vk::Format::eD32Sfloat);

    const auto& bindingDescription    = Vertex::getBindingDescription();
    const auto& attributeDescriptions = Vertex::getAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo
        .setVertexBindingDescriptions(bindingDescription)
        .setVertexAttributeDescriptions(attributeDescriptions);

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
        .setLayout(pipelineLayout)
        .setRenderPass(nullptr);

    VK_CREATE(_device.createGraphicsPipeline(nullptr, pipelineInfo), pipeline, errorMessage);

    return true;
}
