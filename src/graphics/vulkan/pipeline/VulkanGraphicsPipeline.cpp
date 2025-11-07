#include "VulkanGraphicsPipeline.h"

#include "graphics/vulkan/common/VulkanDebugger.h"
#include "graphics/vulkan/resources/mesh/VulkanMesh.h"

bool VulkanGraphicsPipeline::create(
    const vk::Device&                           device,
    const VulkanSwapchain&                      swapchain,
    const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
    const VulkanShaderProgram&                  shaderProgram,
    std::string&                                errorMessage
) noexcept {
    _device    = device;
    _swapchain = &swapchain;

    if (!createPipelineLayout(descriptorSetLayouts, errorMessage)) return false;
    if (!createPipeline(shaderProgram, errorMessage))              return false;

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
    const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts, std::string& errorMessage) {
    vk::PipelineLayoutCreateInfo layoutInfo{};
    layoutInfo
        .setSetLayouts(descriptorSetLayouts)
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

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

    if (!shaderProgram.isFullscreen()) {
        const auto& bindingDescription    = Vertex::getBindingDescription();
        const auto& attributeDescriptions = Vertex::getAttributeDescriptions();

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
        .setLayout(pipelineLayout)
        .setRenderPass(nullptr);

    VK_CREATE(_device.createGraphicsPipeline(nullptr, pipelineInfo), pipeline, errorMessage);

    return true;
}
