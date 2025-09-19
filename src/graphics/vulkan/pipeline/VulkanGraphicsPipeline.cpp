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

    if (!createPipelineLayout(errorMessage)) return false;
    if (!createPipeline(errorMessage))       return false;

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

    VK_CREATE(_device->createPipelineLayout(layoutInfo), pipelineLayout, errorMessage);
    return true;
}

bool VulkanGraphicsPipeline::createPipeline(std::string& errorMessage) {
        VulkanShaderProgram meowProgram(_device);
    if (!meowProgram.loadFromFiles({"../../shaders/spv/meow.vert.spv", "../../shaders/spv/meow.frag.spv"}, errorMessage)) {
        return false;
    }

    const auto stages = meowProgram.getStages();

    const vk::Format colorFormat = _swapchain->getFormat();

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

    VK_CREATE(_device->createGraphicsPipeline(nullptr, pipelineInfo), pipeline, errorMessage);
    return true;
}
