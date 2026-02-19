#include "VulkanGraphicsPipeline.h"

#include "graphics/vulkan/common/VulkanDebugger.h"
#include "graphics/vulkan/rendergraph/VulkanRenderResources.h"
#include "graphics/vulkan/resources/meshes/VulkanVertex.h"

#include "core/debug/ErrorHandling.h"

#include <ranges>

bool VulkanGraphicsPipeline::create(
    const vk::Device&       device,
    const VulkanRenderPass& pass,
    std::string&            errorMessage
) noexcept {
    _device = device;

    TRY(createPipelineLayout(
        device, pass.getPipelineDescriptor(), errorMessage
    ));

    TRY(createPipeline(device, pass, errorMessage));

    return true;
}

void VulkanGraphicsPipeline::destroy() noexcept {
    if (!_device) return;

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
    const vk::Device&               device,
    const VulkanPipelineDescriptor& descriptor,
    std::string&                    errorMessage
) {
    std::vector<vk::PushConstantRange> _pushConstantRanges{};

    for (const auto& [stageFlags, offset, size] : descriptor.shaderProgram->getPushConstants() | std::views::values) {
        _pushConstantRanges.emplace_back(stageFlags, offset, size);
    }

    vk::PipelineLayoutCreateInfo layoutInfo{};
    layoutInfo
        .setSetLayouts(descriptor.descriptorLayouts)
        .setPushConstantRanges(_pushConstantRanges);

    VK_CREATE(device.createPipelineLayout(layoutInfo), _pipelineLayout, errorMessage);

    return true;
}

/*---------------------------------------*/
/*        Pipeline State Structs         */
/*---------------------------------------*/

namespace {

vk::PipelineViewportStateCreateInfo makeViewportState() noexcept {
    vk::PipelineViewportStateCreateInfo info{};
    info
        .setViewportCount(1)
        .setScissorCount(1)
        .setPViewports(nullptr)
        .setPScissors(nullptr);
    return info;
}

vk::PipelineMultisampleStateCreateInfo makeMultisampleState() noexcept {
    vk::PipelineMultisampleStateCreateInfo info{};
    info
        .setRasterizationSamples(vk::SampleCountFlagBits::e1)
        .setSampleShadingEnable(vk::False);
    return info;
}

vk::PipelineColorBlendAttachmentState makeColorBlendAttachmentState() noexcept {
    vk::PipelineColorBlendAttachmentState info{};
    info
        .setColorWriteMask(
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        )
        .setBlendEnable(vk::False)
        .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
        .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
        .setColorBlendOp(vk::BlendOp::eAdd)
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
        .setAlphaBlendOp(vk::BlendOp::eAdd);
    return info;
}

vk::PipelineDynamicStateCreateInfo makeDynamicState() noexcept {
    static constexpr std::array dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo info{};
    info.setDynamicStates(dynamicStates);
    return info;
}

}

bool VulkanGraphicsPipeline::createPipeline(
    const vk::Device&       device,
    const VulkanRenderPass& pass,
    std::string&            errorMessage
) {
    const auto& shaderStages = pass.getPipelineDescriptor().shaderProgram->getStages();

    // Color attachment state

    const auto colorBlendAttachmentState = makeColorBlendAttachmentState();

    std::vector<vk::Format>                            colorAttachmentFormats{};
    std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments{};

    const auto& colorAttachments = pass.getColorAttachments();

    colorAttachmentFormats.reserve(colorAttachments.size());
    colorBlendAttachments.reserve(colorAttachments.size());

    for (const auto& colorAttachment : colorAttachments) {
        colorAttachmentFormats.push_back(colorAttachment->resource.resolveImage()->getFormat());
        colorBlendAttachments.push_back(colorBlendAttachmentState);
    }

    // Rendering info

    vk::PipelineRenderingCreateInfo renderingInfo{};
    renderingInfo
        .setColorAttachmentFormats(colorAttachmentFormats)
        .setDepthAttachmentFormat(VulkanRenderResources::DEPTH_BUFFER_FORMAT);

    // Vertex input state

    vk::PipelineVertexInputStateCreateInfo vertexInputState{};

    const auto& bindingDescription    = VulkanVertex::getBindingDescription();
    const auto& attributeDescriptions = VulkanVertex::getAttributeDescriptions();

    if (pass.getType() == VulkanRenderPassType::MeshRender || pass.getType() == VulkanRenderPassType::Debug) {
        vertexInputState
            .setVertexBindingDescriptions(bindingDescription)
            .setVertexAttributeDescriptions(attributeDescriptions);
    }

    // Input assembly state

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState{};
    inputAssemblyState.setPrimitiveRestartEnable(vk::False);

    if (pass.getType() == VulkanRenderPassType::Debug) {
        inputAssemblyState.setTopology(vk::PrimitiveTopology::eLineList);
    } else {
        inputAssemblyState.setTopology(vk::PrimitiveTopology::eTriangleList);
    }

    // Viewport state

    const auto viewportState = makeViewportState();

    // Rasterization state

    vk::PipelineRasterizationStateCreateInfo rasterizationState{};
    rasterizationState
        .setDepthClampEnable(vk::False)
        .setRasterizerDiscardEnable(vk::False)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setFrontFace(vk::FrontFace::eCounterClockwise)
        .setDepthBiasEnable(vk::False)
        .setLineWidth(1.0f);

    switch (pass.getType()) {
        case VulkanRenderPassType::MeshRender:
            rasterizationState.setCullMode(vk::CullModeFlagBits::eBack);
            break;
        case VulkanRenderPassType::Composite:
            rasterizationState.setCullMode(vk::CullModeFlagBits::eNone);
            break;
        case VulkanRenderPassType::Debug:
            rasterizationState
                .setCullMode(vk::CullModeFlagBits::eBack)
                .setPolygonMode(vk::PolygonMode::eLine);
            break;
        default:
            errorMessage = "Failed to create Vulkan graphics pipeline: unknown pass type.";
            return false;
    }

    // Multisample state

    const auto multisampleState = makeMultisampleState();

    // Depth stencil state

    vk::PipelineDepthStencilStateCreateInfo depthStencilState{};
    depthStencilState
        .setDepthTestEnable(vk::True)
        .setDepthWriteEnable(vk::True)
        .setDepthCompareOp(vk::CompareOp::eLessOrEqual)
        .setDepthBoundsTestEnable(vk::False)
        .setStencilTestEnable(vk::False);

    if (pass.getType() == VulkanRenderPassType::Debug) {
        depthStencilState.setDepthWriteEnable(vk::False);
    }

    // Color blend state

    vk::PipelineColorBlendStateCreateInfo colorBlendState{};
    colorBlendState
        .setLogicOpEnable(vk::False)
        .setLogicOp(vk::LogicOp::eCopy)
        .setAttachments(colorBlendAttachments);

    // Dynamic state

    const auto dynamicState = makeDynamicState();

    // Pipeline info

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo
        .setPNext(&renderingInfo)
        .setStages(shaderStages)
        .setPVertexInputState(&vertexInputState)
        .setPInputAssemblyState(&inputAssemblyState)
        .setPViewportState(&viewportState)
        .setPRasterizationState(&rasterizationState)
        .setPMultisampleState(&multisampleState)
        .setPDepthStencilState(&depthStencilState)
        .setPColorBlendState(&colorBlendState)
        .setPDynamicState(&dynamicState)
        .setLayout(_pipelineLayout)
        .setRenderPass(nullptr);

    VK_CREATE(device.createGraphicsPipeline(nullptr, pipelineInfo), _pipeline, errorMessage);

    return true;
}
