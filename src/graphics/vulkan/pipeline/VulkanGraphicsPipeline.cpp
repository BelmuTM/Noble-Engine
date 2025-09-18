#include "VulkanGraphicsPipeline.h"
#include "VulkanShaderProgram.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

static const std::vector dynamicStates = {
    vk::DynamicState::eViewport,
    vk::DynamicState::eScissor
};

bool VulkanGraphicsPipeline::create(
    const vk::Device* device, const VulkanSwapchain& swapchain, std::string& errorMessage
) noexcept {
    _device    = device;
    _swapchain = &swapchain;

    if (!createPipelineLayout(errorMessage)) { return false; }
    if (!createPipeline(errorMessage))       { return false; }

    return true;
}

void VulkanGraphicsPipeline::destroy() noexcept {
    if (pipelineLayout) {
        _device->destroyPipelineLayout(pipelineLayout);
        pipelineLayout = nullptr;
    }

    if (pipeline) {
        _device->destroyPipeline(pipeline);
        pipeline = nullptr;
    }

    _device    = nullptr;
    _swapchain = nullptr;
}

bool VulkanGraphicsPipeline::createPipelineLayout(std::string& errorMessage) {
    vk::PipelineLayoutCreateInfo layoutInfo{};
    layoutInfo
        .setSetLayoutCount(0)
        .setPushConstantRangeCount(0);

    const auto pipelineLayoutCreate = VK_CHECK_RESULT(_device->createPipelineLayout(layoutInfo), errorMessage);
    if (pipelineLayoutCreate.result != vk::Result::eSuccess) {
        return false;
    }
    pipelineLayout = pipelineLayoutCreate.value;
    return true;
}

bool VulkanGraphicsPipeline::createPipeline(std::string& errorMessage) {
        VulkanShaderProgram meowProgram(_device);
    if (!meowProgram.loadFromFiles({"../../shaders/spv/meow.vert.spv", "../../shaders/spv/meow.frag.spv"}, errorMessage)) {
        return false;
    }

    const auto stages = meowProgram.getStages();

    vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.setDynamicStates(dynamicStates);

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.setTopology(vk::PrimitiveTopology::eTriangleList);

    // TO-DO: write JSON pipeline config loader for vk boilerplate

    vk::PipelineViewportStateCreateInfo viewportInfo{};
    viewportInfo
        .setViewportCount(1)
        .setScissorCount(1);

    vk::PipelineRasterizationStateCreateInfo rasterizationInfo{};
    rasterizationInfo
        .setDepthClampEnable(vk::False)
        .setRasterizerDiscardEnable(vk::False)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setCullMode(vk::CullModeFlagBits::eNone)
        .setFrontFace(vk::FrontFace::eClockwise)
        .setDepthBiasEnable(vk::False)
        .setLineWidth(1.0f);

    vk::PipelineMultisampleStateCreateInfo multisamplingInfo{};
    multisamplingInfo
        .setRasterizationSamples(vk::SampleCountFlagBits::e1)
        .setSampleShadingEnable(vk::False);

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                           vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
        .setBlendEnable(vk::True)
        .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
        .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
        .setColorBlendOp(vk::BlendOp::eAdd)
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
        .setAlphaBlendOp(vk::BlendOp::eAdd);

    vk::PipelineColorBlendStateCreateInfo colorBlendInfo{};
    colorBlendInfo
        .setLogicOpEnable(vk::False)
        .setLogicOp(vk::LogicOp::eCopy)
        .setAttachmentCount(1)
        .setAttachments(colorBlendAttachment);

    vk::Format colorFormat = _swapchain->getFormat();

    vk::PipelineRenderingCreateInfo renderingInfo{};
    renderingInfo
        .setColorAttachmentCount(1)
        .setPColorAttachmentFormats(&colorFormat);

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo
        .setPNext(&renderingInfo)
        .setStageCount(stages.size())
        .setStages(stages)
        .setPVertexInputState(&vertexInputInfo)
        .setPInputAssemblyState(&inputAssemblyInfo)
        .setPViewportState(&viewportInfo)
        .setPRasterizationState(&rasterizationInfo)
        .setPMultisampleState(&multisamplingInfo)
        .setPColorBlendState(&colorBlendInfo)
        .setPDynamicState(&dynamicStateInfo)
        .setLayout(pipelineLayout)
        .setRenderPass(nullptr);

    const auto pipelineCreate = VK_CHECK_RESULT(_device->createGraphicsPipeline(nullptr, pipelineInfo), errorMessage);
    if (pipelineCreate.result != vk::Result::eSuccess) {
        return false;
    }
    pipeline = pipelineCreate.value;

    return true;
}
