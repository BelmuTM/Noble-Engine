#include "VulkanGraphicsPipeline.h"
#include "VulkanShaderProgram.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

// TO-DO: reuse global staging buffer pool

bool VulkanGraphicsPipeline::create(
    const VulkanDevice& device, const VulkanSwapchain& swapchain, const VulkanCommandManager& commandManager,
    std::string& errorMessage
) noexcept {
    _device         = &device;
    _swapchain      = &swapchain;
    _commandManager = &commandManager;

    if (!createPipelineLayout(errorMessage)) return false;
    if (!createPipeline(errorMessage))       return false;
    if (!createStagingBuffer(errorMessage))  return false;
    if (!createVertexBuffer(errorMessage))   return false;
    if (!createIndexBuffer(errorMessage))    return false;

    stagingBuffer.destroy();
    return true;
}

void VulkanGraphicsPipeline::destroy() noexcept {
    const vk::Device& logicalDevice = _device->getLogicalDevice();

    vertexBuffer.destroy();
    indexBuffer.destroy();

    if (pipeline) {
        logicalDevice.destroyPipeline(pipeline);
        pipeline = VK_NULL_HANDLE;
    }

    if (pipelineLayout) {
        logicalDevice.destroyPipelineLayout(pipelineLayout);
        pipelineLayout = VK_NULL_HANDLE;
    }

    _swapchain = nullptr;
}

bool VulkanGraphicsPipeline::createPipelineLayout(std::string& errorMessage) {
    vk::PipelineLayoutCreateInfo layoutInfo{};
    layoutInfo
        .setSetLayoutCount(0)
        .setPushConstantRangeCount(0);

    VK_CREATE(_device->getLogicalDevice().createPipelineLayout(layoutInfo), pipelineLayout, errorMessage);
    return true;
}

bool VulkanGraphicsPipeline::createPipeline(std::string& errorMessage) {
    const vk::Device& logicalDevice = _device->getLogicalDevice();

    VulkanShaderProgram meowProgram(logicalDevice);
    if (!meowProgram.loadFromFiles({"../../shaders/spv/meow.vert.spv", "../../shaders/spv/meow.frag.spv"}, errorMessage)) {
        return false;
    }

    const auto stages = meowProgram.getStages();

    const vk::Format& colorFormat = _swapchain->getFormat();

    vk::PipelineRenderingCreateInfo renderingInfo{};
    renderingInfo
        .setColorAttachmentCount(1)
        .setPColorAttachmentFormats(&colorFormat);

    const auto& bindingDescription    = Vertex::getBindingDescription();
    const auto& attributeDescriptions = Vertex::getAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo
        .setVertexBindingDescriptionCount(1)
        .setPVertexBindingDescriptions(&bindingDescription)
        .setVertexAttributeDescriptionCount(attributeDescriptions.size())
        .setPVertexAttributeDescriptions(attributeDescriptions.data());

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

    VK_CREATE(logicalDevice.createGraphicsPipeline(nullptr, pipelineInfo), pipeline, errorMessage);
    return true;
}

bool VulkanGraphicsPipeline::createStagingBuffer(std::string& errorMessage) {
    const vk::DeviceSize stagingBufferSize = vertexBufferSize + indexBufferSize;

    if (!stagingBuffer.create(
        stagingBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        _device,
        errorMessage
    )) {
        return false;
    }

    // Mapping GPU allocated memory to CPU memory
    void* stagingData = stagingBuffer.mapMemory(errorMessage);
    if (!stagingData) return false;

    // Copying both vertex and index data into CPU memory using aliasing
    memcpy(stagingData, vertices.data(), vertexBufferSize);
    memcpy(static_cast<char*>(stagingData) + vertexBufferSize, indices.data(), indexBufferSize);

    stagingBuffer.unmapMemory();
    return true;
}

bool VulkanGraphicsPipeline::createVertexBuffer(std::string& errorMessage) {
    if (!vertexBuffer.create(
        vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        _device,
        errorMessage
    )) {
        return false;
    }

    if (!vertexBuffer.copyFrom(stagingBuffer, _commandManager, errorMessage))
        return false;
    return true;
}

bool VulkanGraphicsPipeline::createIndexBuffer(std::string& errorMessage) {
    if (!indexBuffer.create(
        indexBufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        _device,
        errorMessage
    )) {
        return false;
    }

    if (!indexBuffer.copyFrom(stagingBuffer, _commandManager, errorMessage, indexBufferSize, vertexBufferSize, 0))
        return false;
    return true;
}
