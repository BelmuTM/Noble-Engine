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
    if (!createPipelineLayout(device, descriptorSetLayouts, errorMessage)) return false;
    if (!createPipeline(device, swapchain, shaderProgram, errorMessage))   return false;

    return true;
}

void VulkanGraphicsPipeline::destroy(const vk::Device& device) noexcept {
    if (pipeline) {
        device.destroyPipeline(pipeline);
        pipeline = VK_NULL_HANDLE;
    }

    if (pipelineLayout) {
        device.destroyPipelineLayout(pipelineLayout);
        pipelineLayout = VK_NULL_HANDLE;
    }
}

bool VulkanGraphicsPipeline::createPipelineLayout(
    const vk::Device&                           device,
    const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
    std::string&                                errorMessage
) {
    vk::PipelineLayoutCreateInfo layoutInfo{};
    layoutInfo
        .setSetLayouts(descriptorSetLayouts)
        .setPushConstantRangeCount(0);

    VK_CREATE(device.createPipelineLayout(layoutInfo), pipelineLayout, errorMessage);

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

    VK_CREATE(device.createGraphicsPipeline(nullptr, pipelineInfo), pipeline, errorMessage);

    return true;
}
