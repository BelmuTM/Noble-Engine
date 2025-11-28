#include "VulkanGraphicsPipeline.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/resources/meshes/VulkanVertex.h"

#include "core/debug/ErrorHandling.h"

#include <ranges>

bool VulkanGraphicsPipeline::create(
    const vk::Device&               device,
    const VulkanPipelineDescriptor& descriptor,
    const AttachmentsVector&        colorAttachments,
    std::string&                    errorMessage
) noexcept {
    _device = device;

    TRY(createPipelineLayout(
        device, descriptor.descriptorLayouts, descriptor.shaderProgram->getPushConstants(), errorMessage
    ));

    TRY(createPipeline(device, *descriptor.shaderProgram, colorAttachments, errorMessage));

    _stageFlags = descriptor.shaderProgram->getStageFlags();

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
    const PushConstantsMap&                     pushConstantRanges,
    std::string&                                errorMessage
) {
    std::vector<vk::PushConstantRange> _pushConstantRanges{};

    for (const auto& [stageFlags, offset, size] : pushConstantRanges | std::views::values) {
        _pushConstantRanges.emplace_back(stageFlags, offset, size);
    }

    vk::PipelineLayoutCreateInfo layoutInfo{};
    layoutInfo
        .setSetLayouts(descriptorSetLayouts)
        .setPushConstantRanges(_pushConstantRanges);

    VK_CREATE(device.createPipelineLayout(layoutInfo), _pipelineLayout, errorMessage);

    return true;
}

bool VulkanGraphicsPipeline::createPipeline(
    const vk::Device&          device,
    const VulkanShaderProgram& shaderProgram,
    const AttachmentsVector&   colorAttachments,
    std::string&               errorMessage
) {
    std::vector<vk::Format>                            colorAttachmentFormats{};
    std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments{};

    for (const auto& colorAttachment : colorAttachments) {
        colorAttachmentFormats.push_back(colorAttachment->resource.resolveImage()->getFormat());
        colorBlendAttachments.push_back(colorBlendAttachment);
    }

    vk::PipelineRenderingCreateInfo renderingInfo{};
    renderingInfo
        .setColorAttachmentFormats(colorAttachmentFormats)
        .setDepthAttachmentFormat(vk::Format::eD32Sfloat);

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

    const auto& bindingDescription    = VulkanVertex::getBindingDescription();
    const auto& attributeDescriptions = VulkanVertex::getAttributeDescriptions();

    if (!shaderProgram.isFullscreen()) {
        vertexInputInfo
            .setVertexBindingDescriptions(bindingDescription)
            .setVertexAttributeDescriptions(attributeDescriptions);
    }

    vk::PipelineRasterizationStateCreateInfo rasterizationInfo{};
    rasterizationInfo
        .setDepthClampEnable(vk::False)
        .setRasterizerDiscardEnable(vk::False)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setFrontFace(vk::FrontFace::eCounterClockwise)
        .setDepthBiasEnable(vk::False)
        .setLineWidth(1.0f);

    if (shaderProgram.isFullscreen()) {
        rasterizationInfo.setCullMode(vk::CullModeFlagBits::eNone);
    } else {
        rasterizationInfo.setCullMode(vk::CullModeFlagBits::eBack);
    }

    vk::PipelineColorBlendStateCreateInfo colorBlendInfo{};
    colorBlendInfo
        .setLogicOpEnable(vk::False)
        .setLogicOp(vk::LogicOp::eCopy)
        .setAttachments(colorBlendAttachments);

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
