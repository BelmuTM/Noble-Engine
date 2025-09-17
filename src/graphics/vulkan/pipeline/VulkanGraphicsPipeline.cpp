#include "VulkanGraphicsPipeline.h"
#include "VulkanShaderProgram.h"

#include "../common/VulkanDebugger.h"

static const std::vector dynamicStates = {
    vk::DynamicState::eViewport,
    vk::DynamicState::eScissor
};

bool VulkanGraphicsPipeline::create(
    const vk::Device* device, const VulkanSwapchain& swapchain, std::string& errorMessage
) noexcept {
    _device    = device;
    _swapchain = &swapchain;

    VulkanShaderProgram meowProgram(_device);
    if (!meowProgram.loadFromFiles({"../../shaders/spv/meow.vert.spv", "../../shaders/spv/meow.frag.spv"}, errorMessage)) {
        return false;
    }

    const auto stages = meowProgram.getStages();

    vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.setDynamicStates(dynamicStates);

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;

    const vk::Extent2D extent = swapchain.getExtent2D();

    vk::Viewport viewport = {
        0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f
    };

    vk::PipelineViewportStateCreateInfo viewportInfo{};
    viewportInfo.viewportCount = 1;
    viewportInfo.scissorCount  = 1;

    vk::PipelineRasterizationStateCreateInfo rasterizationInfo{};
    rasterizationInfo
        .setDepthClampEnable(VK_FALSE)
        .setRasterizerDiscardEnable(VK_FALSE)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setCullMode(vk::CullModeFlagBits::eBack)
        .setFrontFace(vk::FrontFace::eClockwise)
        .setDepthBiasEnable(VK_FALSE)
        .setLineWidth(1.0f);

    vk::PipelineMultisampleStateCreateInfo multisamplingInfo{};
    multisamplingInfo
        .setRasterizationSamples(vk::SampleCountFlagBits::e1)
        .setSampleShadingEnable(vk::False);

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment
        .setBlendEnable(VK_TRUE)
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

    vk::PipelineLayoutCreateInfo layoutInfo{};
    layoutInfo
        .setSetLayoutCount(0)
        .setPushConstantRangeCount(0);

    const auto pipelineLayoutCreate = VK_CHECK_RESULT(_device->createPipelineLayout(layoutInfo), errorMessage);
    if (pipelineLayoutCreate.result != vk::Result::eSuccess) {
        return false;
    }
    pipelineLayout = pipelineLayoutCreate.value;

    vk::PipelineRenderingCreateInfo renderingInfo{};
    renderingInfo
        .setColorAttachmentCount(1)
        .setPColorAttachmentFormats(_swapchain->getFormat());

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

void VulkanGraphicsPipeline::destroy() noexcept {
    if (pipelineLayout) {
        _device->destroyPipelineLayout(pipelineLayout);
        pipelineLayout = nullptr;
    }

    if (pipeline) {
        _device->destroyPipeline(pipeline);
        _device    = nullptr;
        _swapchain = nullptr;
        pipeline   = nullptr;
    }
}
