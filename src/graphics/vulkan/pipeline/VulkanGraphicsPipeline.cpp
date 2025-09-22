#include "VulkanGraphicsPipeline.h"
#include "VulkanShaderProgram.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

const std::vector<Vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

bool VulkanGraphicsPipeline::create(
    const VulkanDevice& device, const VulkanSwapchain& swapchain, std::string& errorMessage
) noexcept {
    _device    = &device;
    _swapchain = &swapchain;

    if (!createPipelineLayout(errorMessage)) return false;
    if (!createPipeline(errorMessage))       return false;
    if (!createVertexBuffer(errorMessage))   return false;
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
   vk::Buffer& buffer, vk::DeviceMemory& bufferMemory, const vk::DeviceSize size, const vk::BufferUsageFlags usage,
   const vk::MemoryPropertyFlags properties, std::string& errorMessage
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

bool VulkanGraphicsPipeline::createVertexBuffer(std::string& errorMessage) {
    const vk::Device&     logicalDevice = _device->getLogicalDevice();
    const vk::DeviceSize& bufferSize    = sizeof(vertices[0]) * vertices.size();

    const vk::MemoryPropertyFlags& properties =
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

    if (!createBuffer(vertexBuffer, vertexBufferMemory, bufferSize, vk::BufferUsageFlagBits::eVertexBuffer, properties,
                      errorMessage)) return false;

    // Mapping GPU allocated memory to CPU memory
    void* data = nullptr;
    VK_CREATE(logicalDevice.mapMemory(vertexBufferMemory, 0, bufferSize), data, errorMessage);
    memcpy(data, vertices.data(), bufferSize);
    logicalDevice.unmapMemory(vertexBufferMemory);

    return true;
}
