#include "VulkanBuffer.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept {
    _device       = other._device;
    _buffer       = other._buffer;
    _bufferSize   = other._bufferSize;
    _bufferMemory = other._bufferMemory;

    other._device       = nullptr;
    other._buffer       = VK_NULL_HANDLE;
    other._bufferSize   = 0;
    other._bufferMemory = VK_NULL_HANDLE;
}

VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& other) noexcept {
    if (this != &other) {
        destroy();

        _device       = other._device;
        _buffer       = other._buffer;
        _bufferSize   = other._bufferSize;
        _bufferMemory = other._bufferMemory;

        other._device       = nullptr;
        other._buffer       = VK_NULL_HANDLE;
        other._bufferSize   = 0;
        other._bufferMemory = VK_NULL_HANDLE;
    }
    return *this;
}

bool VulkanBuffer::create(
    const vk::DeviceSize          size,
    const vk::BufferUsageFlags    usage,
    const vk::MemoryPropertyFlags properties,
    const VulkanDevice*           device,
    std::string&                  errorMessage
) noexcept {
    _device = device;

    if (!createBuffer(_buffer, _bufferMemory, size, usage, properties, device, errorMessage)) return false;

    _bufferSize = size;
    return true;
}

void VulkanBuffer::destroy() noexcept {
    if (!_device) return;

    const vk::Device& logicalDevice = _device->getLogicalDevice();

    if (_buffer) {
        logicalDevice.destroyBuffer(_buffer);
        _buffer = VK_NULL_HANDLE;
    }

    if (_bufferMemory) {
        logicalDevice.freeMemory(_bufferMemory);
        _bufferMemory = VK_NULL_HANDLE;
    }

    _device = nullptr;
}

bool VulkanBuffer::createBuffer(
    vk::Buffer&                   buffer,
    vk::DeviceMemory&             bufferMemory,
    const vk::DeviceSize          size,
    const vk::BufferUsageFlags    usage,
    const vk::MemoryPropertyFlags properties,
    const VulkanDevice*           device,
    std::string&                  errorMessage
) {
    const vk::Device& logicalDevice = device->getLogicalDevice();

    vk::BufferCreateInfo bufferInfo{};
    bufferInfo
        .setSize(size)
        .setUsage(usage)
        .setSharingMode(vk::SharingMode::eExclusive);

    VK_CREATE(logicalDevice.createBuffer(bufferInfo), buffer, errorMessage);

    const vk::MemoryRequirements& memoryRequirements = logicalDevice.getBufferMemoryRequirements(buffer);

    const uint32_t memoryTypeIndex = device->findMemoryType(memoryRequirements.memoryTypeBits, properties);

    vk::MemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo
        .setAllocationSize(memoryRequirements.size)
        .setMemoryTypeIndex(memoryTypeIndex);

    VK_CREATE(logicalDevice.allocateMemory(memoryAllocateInfo), bufferMemory, errorMessage);
    VK_CALL(logicalDevice.bindBufferMemory(buffer, bufferMemory, 0), errorMessage);
    return true;
}

bool VulkanBuffer::copyBuffer(
    vk::Buffer&                 srcBuffer,
    vk::Buffer&                 dstBuffer,
    vk::DeviceSize              size,
    vk::DeviceSize              srcOffset,
    vk::DeviceSize              dstOffset,
    const VulkanDevice*         device,
    const VulkanCommandManager* commandManager,
    std::string&                errorMessage
) {
    vk::CommandBuffer copyCommandBuffer;
    if(!commandManager->createCommandBuffer(copyCommandBuffer, errorMessage)) return false;

    constexpr vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
    VK_TRY(copyCommandBuffer.begin(beginInfo), errorMessage);

    copyCommandBuffer.copyBuffer(srcBuffer, dstBuffer, vk::BufferCopy(srcOffset, dstOffset, size));

    VK_TRY(copyCommandBuffer.end(), errorMessage);

    const vk::Queue& graphicsQueue = device->getGraphicsQueue();

    vk::SubmitInfo submitInfo{};
    submitInfo
        .setCommandBufferCount(1)
        .setCommandBuffers(copyCommandBuffer);

    VK_TRY(graphicsQueue.submit(submitInfo), errorMessage);
    VK_TRY(graphicsQueue.waitIdle(), errorMessage);
    return true;
}

bool VulkanBuffer::copyFrom(
    vk::Buffer&                 srcBuffer,
    const VulkanCommandManager* commandManager,
    std::string&                errorMessage,
    vk::DeviceSize              size,
    vk::DeviceSize              srcOffset,
    vk::DeviceSize              dstOffset
) {
    if (size == VK_WHOLE_SIZE) size = _bufferSize;

    if (!copyBuffer(srcBuffer, _buffer, size, srcOffset, dstOffset, _device, commandManager, errorMessage))
        return false;
    return true;
}

void* VulkanBuffer::mapMemory(std::string& errorMessage, vk::DeviceSize offset, vk::DeviceSize size) {
    if (!_device || !_bufferMemory) {
        errorMessage = "Failed to map buffer memory: device or memory not initialized";
        return nullptr;
    }

    if (size == VK_WHOLE_SIZE) size = _bufferSize;

    const auto memoryMap = VK_CALL(_device->getLogicalDevice().mapMemory(_bufferMemory, offset, size), errorMessage);
    if (memoryMap.result != vk::Result::eSuccess) {
        return nullptr;
    }
    _mappedPointer = memoryMap.value;
    return _mappedPointer;
}

void VulkanBuffer::unmapMemory() {
    if (_device && _bufferMemory && _mappedPointer) {
        _device->getLogicalDevice().unmapMemory(_bufferMemory);
        _mappedPointer = nullptr;
    }
}
