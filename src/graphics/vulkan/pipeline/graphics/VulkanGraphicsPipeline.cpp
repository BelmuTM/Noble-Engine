#include "VulkanGraphicsPipeline.h"

#include "graphics/vulkan/common/VulkanDebugger.h"
#include "graphics/vulkan/rendergraph/resources/VulkanRenderResourceManager.h"

#include "graphics/vulkan/resources/meshes/VulkanVertex.h"

Expected<void> VulkanGraphicsPipeline::create(
    const vk::Device& device, const VulkanGraphicsPipelineDescriptor& descriptor
) noexcept {
    _device = device;

    TRY(createPipelineLayout(device, descriptor.layout));
    TRY(createGraphicsPipeline(device, descriptor));

    return {};
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

Expected<void> VulkanGraphicsPipeline::createGraphicsPipeline(
    const vk::Device& device, const VulkanGraphicsPipelineDescriptor& descriptor
) {
    // Color attachment state

    const auto colorBlendAttachmentState = makeColorBlendAttachmentState();

    std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments{};
    colorBlendAttachments.reserve(descriptor.colorAttachmentFormats.size());

    for (const auto& _ : descriptor.colorAttachmentFormats) {
        colorBlendAttachments.push_back(colorBlendAttachmentState);
    }

    // Rendering info

    vk::PipelineRenderingCreateInfo renderingInfo{};
    renderingInfo
        .setColorAttachmentFormats(descriptor.colorAttachmentFormats)
        .setDepthAttachmentFormat(descriptor.depthAttachmentFormat);

    // Vertex input state

    vk::PipelineVertexInputStateCreateInfo vertexInputState{};

    const auto& bindingDescription    = VulkanVertex::getBindingDescription();
    const auto& attributeDescriptions = VulkanVertex::getAttributeDescriptions();

    if (descriptor.passType == VulkanGraphicsPassType::MeshRender || descriptor.passType == VulkanGraphicsPassType::Debug) {
        vertexInputState
            .setVertexBindingDescriptions(bindingDescription)
            .setVertexAttributeDescriptions(attributeDescriptions);
    }

    // Input assembly state

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState{};
    inputAssemblyState.setPrimitiveRestartEnable(vk::False);

    if (descriptor.passType == VulkanGraphicsPassType::Debug) {
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
        .setLineWidth(2.0f);

    switch (descriptor.passType) {
        case VulkanGraphicsPassType::MeshRender:
            rasterizationState.setCullMode(vk::CullModeFlagBits::eBack);
            break;
        case VulkanGraphicsPassType::Composite:
            rasterizationState.setCullMode(vk::CullModeFlagBits::eNone);
            break;
        case VulkanGraphicsPassType::Debug:
            rasterizationState
                .setCullMode(vk::CullModeFlagBits::eBack)
                .setPolygonMode(vk::PolygonMode::eLine);
            break;
        default:
            return VK_FAIL("Failed to create graphics pipeline: unknown pass type.");
    }

    // Multisample state

    const auto multisampleState = makeMultisampleState();

    // Depth stencil state

    vk::PipelineDepthStencilStateCreateInfo depthStencilState{};
    depthStencilState
        .setDepthTestEnable(vk::True)
        .setDepthWriteEnable(vk::True)
        .setDepthCompareOp(vk::CompareOp::eGreaterOrEqual)
        .setDepthBoundsTestEnable(vk::False)
        .setStencilTestEnable(vk::False);

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
        .setStages(descriptor.shaderStages)
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

    VK_CREATE(_pipeline, device.createGraphicsPipeline(nullptr, pipelineInfo));

    return {};
}
