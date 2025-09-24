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

    destroyStagingBuffer();
    return true;
}

void VulkanGraphicsPipeline::destroy() noexcept {
    const vk::Device& logicalDevice = _device->getLogicalDevice();

    if (vertexBuffer) {
        logicalDevice.destroyBuffer(vertexBuffer);
        vertexBuffer = VK_NULL_HANDLE;
    }

    if (vertexBufferMemory) {
        logicalDevice.freeMemory(vertexBufferMemory);
        vertexBufferMemory = VK_NULL_HANDLE;
    }

    if (indexBuffer) {
        logicalDevice.destroyBuffer(indexBuffer);
        indexBuffer = VK_NULL_HANDLE;
    }

    if (indexBufferMemory) {
        logicalDevice.freeMemory(indexBufferMemory);
        indexBufferMemory = VK_NULL_HANDLE;
    }

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

bool VulkanGraphicsPipeline::createBuffer(
    vk::Buffer& buffer,
    vk::DeviceMemory& bufferMemory,
    const vk::DeviceSize size,
    const vk::BufferUsageFlags usage,
    const vk::MemoryPropertyFlags properties,
    std::string& errorMessage
) const {
    const vk::Device& logicalDevice = _device->getLogicalDevice();

    vk::BufferCreateInfo bufferInfo{};
    bufferInfo
        .setSize(size)
        .setUsage(usage)
        .setSharingMode(vk::SharingMode::eExclusive);

    VK_CREATE(logicalDevice.createBuffer(bufferInfo), buffer, errorMessage);

    const vk::MemoryRequirements& memoryRequirements = logicalDevice.getBufferMemoryRequirements(buffer);

    const uint32_t memoryTypeIndex = _device->findMemoryType(memoryRequirements.memoryTypeBits, properties);

    vk::MemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo
        .setAllocationSize(memoryRequirements.size)
        .setMemoryTypeIndex(memoryTypeIndex);

    VK_CREATE(logicalDevice.allocateMemory(memoryAllocateInfo), bufferMemory, errorMessage);
    VK_CALL(logicalDevice.bindBufferMemory(buffer, bufferMemory, 0), errorMessage);
    return true;
}

bool VulkanGraphicsPipeline::copyBuffer(
    vk::Buffer& srcBuffer,
    vk::Buffer& dstBuffer,
    vk::DeviceSize size,
    vk::DeviceSize srcOffset,
    vk::DeviceSize dstOffset,
    std::string& errorMessage
) const {
    vk::CommandBuffer copyCommandBuffer;
    if(!_commandManager->createCommandBuffer(copyCommandBuffer, errorMessage)) return false;

    constexpr vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
    VK_TRY(copyCommandBuffer.begin(beginInfo), errorMessage);

    copyCommandBuffer.copyBuffer(srcBuffer, dstBuffer, vk::BufferCopy(srcOffset, dstOffset, size));

    VK_TRY(copyCommandBuffer.end(), errorMessage);

    const vk::Queue& graphicsQueue = _device->getGraphicsQueue();

    vk::SubmitInfo submitInfo{};
    submitInfo
        .setCommandBufferCount(1)
        .setCommandBuffers(copyCommandBuffer);

    VK_TRY(graphicsQueue.submit(submitInfo), errorMessage);
    VK_TRY(graphicsQueue.waitIdle(), errorMessage);
    return true;
}

bool VulkanGraphicsPipeline::createStagingBuffer(std::string& errorMessage) {
    const vk::Device&    logicalDevice     = _device->getLogicalDevice();
    const vk::DeviceSize stagingBufferSize = vertexBufferSize + indexBufferSize;

    if (!createBuffer(
            stagingBuffer,
            stagingBufferMemory,
            stagingBufferSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
            errorMessage
            )) {
        return false;
    }

    // Mapping GPU allocated memory to CPU memory
    void* stagingData = nullptr;
    VK_CREATE(logicalDevice.mapMemory(stagingBufferMemory, 0, stagingBufferSize), stagingData, errorMessage);
    // Copying both vertex and index data into CPU memory using aliasing
    memcpy(stagingData, vertices.data(), vertexBufferSize);
    memcpy(static_cast<char*>(stagingData) + vertexBufferSize, indices.data(), indexBufferSize);
    logicalDevice.unmapMemory(stagingBufferMemory);
    return true;
}

void VulkanGraphicsPipeline::destroyStagingBuffer() {
    const vk::Device& logicalDevice = _device->getLogicalDevice();

    if (stagingBuffer) {
        logicalDevice.destroyBuffer(stagingBuffer);
        stagingBuffer = VK_NULL_HANDLE;
    }

    if (stagingBufferMemory) {
        logicalDevice.freeMemory(stagingBufferMemory);
        stagingBufferMemory = VK_NULL_HANDLE;
    }
}

bool VulkanGraphicsPipeline::createVertexBuffer(std::string& errorMessage) {
    const vk::Device& logicalDevice = _device->getLogicalDevice();

    if (!createBuffer(
        vertexBuffer,
        vertexBufferMemory,
        vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        errorMessage
    )) {
        return false;
    }

    if (!copyBuffer(stagingBuffer, vertexBuffer, vertexBufferSize, 0, 0, errorMessage)) return false;
    return true;
}

bool VulkanGraphicsPipeline::createIndexBuffer(std::string& errorMessage) {
    const vk::Device& logicalDevice = _device->getLogicalDevice();

    if (!createBuffer(
            indexBuffer,
            indexBufferMemory,
            indexBufferSize,
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            errorMessage
            )) {
        return false;
    }

    if (!copyBuffer(stagingBuffer, indexBuffer, indexBufferSize, vertexBufferSize, 0, errorMessage)) return false;
    return true;
}
