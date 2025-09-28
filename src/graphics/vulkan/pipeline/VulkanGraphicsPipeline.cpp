#include "VulkanGraphicsPipeline.h"
#include "VulkanShaderProgram.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

// TO-DO: reuse global staging buffer pool

bool VulkanGraphicsPipeline::create(
    const VulkanDevice&         device,
    const VulkanSwapchain&      swapchain,
    const VulkanCommandManager& commandManager,
    const uint32_t              framesInFlight,
    std::string&                errorMessage
) noexcept {
    _device         = &device;
    _swapchain      = &swapchain;
    _commandManager = &commandManager;

    if (!createDescriptorSetLayout(errorMessage))            return false;
    if (!createPipelineLayout(errorMessage))                 return false;
    if (!createPipeline(errorMessage))                       return false;
    if (!createStagingBuffer(errorMessage))                  return false;
    if (!createVertexBuffer(errorMessage))                   return false;
    if (!createIndexBuffer(errorMessage))                    return false;
    if (!createUniformBuffers(framesInFlight, errorMessage)) return false;
    if (!createDescriptorPool(framesInFlight, errorMessage)) return false;
    if (!createDescriptorSets(framesInFlight, errorMessage)) return false;

    stagingBuffer.destroy();
    return true;
}

void VulkanGraphicsPipeline::destroy() noexcept {
    const vk::Device& logicalDevice = _device->getLogicalDevice();

    if (descriptorPool) {
        logicalDevice.destroyDescriptorPool(descriptorPool);
        descriptorPool = VK_NULL_HANDLE;
    }

    vertexBuffer.destroy();
    indexBuffer.destroy();

    for (auto& uniformBuffer : uniformBuffers) {
        uniformBuffer.destroy();
    }

    if (pipeline) {
        logicalDevice.destroyPipeline(pipeline);
        pipeline = VK_NULL_HANDLE;
    }

    if (pipelineLayout) {
        logicalDevice.destroyPipelineLayout(pipelineLayout);
        pipelineLayout = VK_NULL_HANDLE;
    }

    if (descriptorSetLayout) {
        logicalDevice.destroyDescriptorSetLayout(descriptorSetLayout);
        descriptorSetLayout = VK_NULL_HANDLE;
    }

    _swapchain = nullptr;
}

bool VulkanGraphicsPipeline::createDescriptorSetLayout(std::string& errorMessage) {
    const vk::Device& logicalDevice = _device->getLogicalDevice();

    vk::DescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding
        .setBinding(0)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eAllGraphics);

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.setBindings(uboLayoutBinding);

    VK_CREATE(logicalDevice.createDescriptorSetLayout(descriptorSetLayoutInfo), descriptorSetLayout, errorMessage);
    return true;
}

bool VulkanGraphicsPipeline::createPipelineLayout(std::string& errorMessage) {
    vk::PipelineLayoutCreateInfo layoutInfo{};
    layoutInfo
        .setSetLayouts(descriptorSetLayout)
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
    renderingInfo.setColorAttachmentFormats(colorFormat);

    const auto& bindingDescription    = Vertex::getBindingDescription();
    const auto& attributeDescriptions = Vertex::getAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo
        .setVertexBindingDescriptions(bindingDescription)
        .setVertexAttributeDescriptions(attributeDescriptions);

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo
        .setPNext(&renderingInfo)
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

bool VulkanGraphicsPipeline::createUniformBuffers(const uint32_t framesInFlight, std::string& errorMessage) {
    uniformBuffers.clear();

    for (size_t i = 0; i < framesInFlight; i++) {
        VulkanBuffer uniformBuffer;

        if (!uniformBuffer.create(
                uniformBufferSize,
                vk::BufferUsageFlagBits::eUniformBuffer,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                _device,
                errorMessage
        )) {
            return false;
        }

        if (!uniformBuffer.mapMemory(errorMessage)) return false;

        uniformBuffers.emplace_back(std::move(uniformBuffer));
    }
    return true;
}

bool VulkanGraphicsPipeline::createDescriptorPool(const uint32_t framesInFlight, std::string& errorMessage) {
    const vk::Device& logicalDevice = _device->getLogicalDevice();

    vk::DescriptorPoolSize descriptorPoolSize(vk::DescriptorType::eUniformBuffer, framesInFlight);

    vk::DescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo
        .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
        .setMaxSets(framesInFlight)
        .setPoolSizes(descriptorPoolSize);

    VK_CREATE(logicalDevice.createDescriptorPool(descriptorPoolInfo), descriptorPool, errorMessage);
    return true;
}

bool VulkanGraphicsPipeline::createDescriptorSets(const uint32_t framesInFlight, std::string& errorMessage) {
    const vk::Device& logicalDevice = _device->getLogicalDevice();

    std::vector layouts(framesInFlight, descriptorSetLayout);

    vk::DescriptorSetAllocateInfo descriptorSetInfo{};
    descriptorSetInfo
        .setDescriptorPool(descriptorPool)
        .setSetLayouts(layouts);

    descriptorSets.clear();

    VK_CREATE(logicalDevice.allocateDescriptorSets(descriptorSetInfo), descriptorSets, errorMessage);

    for (size_t i = 0; i < framesInFlight; i++) {
        vk::DescriptorBufferInfo descriptorBufferInfo{};
        descriptorBufferInfo
            .setBuffer(uniformBuffers[i])
            .setOffset(0)
            .setRange(uniformBufferSize);

        vk::WriteDescriptorSet descriptorSetWrite{};
        descriptorSetWrite
            .setDstSet(descriptorSets[i])
            .setDstBinding(0)
            .setDstArrayElement(0)
            .setDescriptorCount(1)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setBufferInfo(descriptorBufferInfo);

        logicalDevice.updateDescriptorSets(descriptorSetWrite, {});
    }
    return true;
}
