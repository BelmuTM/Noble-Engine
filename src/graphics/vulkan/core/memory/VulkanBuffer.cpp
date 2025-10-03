#include "VulkanBuffer.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept {
    _device        = other._device;
    _buffer        = other._buffer;
    _bufferSize    = other._bufferSize;
    _allocation    = other._allocation;
    _mappedPointer = other._mappedPointer;

    other._device        = nullptr;
    other._buffer        = VK_NULL_HANDLE;
    other._bufferSize    = 0;
    other._allocation    = VK_NULL_HANDLE;
    other._mappedPointer = VK_NULL_HANDLE;
}

VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& other) noexcept {
    if (this != &other) {
        destroy();

        _device        = other._device;
        _buffer        = other._buffer;
        _bufferSize    = other._bufferSize;
        _allocation    = other._allocation;
        _mappedPointer = other._mappedPointer;

        other._device        = nullptr;
        other._buffer        = VK_NULL_HANDLE;
        other._bufferSize    = 0;
        other._allocation    = VK_NULL_HANDLE;
        other._mappedPointer = VK_NULL_HANDLE;
    }
    return *this;
}

bool VulkanBuffer::create(
    const vk::DeviceSize       size,
    const vk::BufferUsageFlags usage,
    const VmaMemoryUsage       memoryUsage,
    const VulkanDevice*        device,
    std::string&               errorMessage
) noexcept {
    _device = device;

    if (!createBuffer(_buffer, _allocation, size, usage, memoryUsage, device, errorMessage)) return false;

    _bufferSize = size;
    return true;
}

void VulkanBuffer::destroy() noexcept {
    if (!_device) return;

    const VmaAllocator allocator = _device->getAllocator();

    if (allocator && _buffer) {
        unmapMemory();
        vmaDestroyBuffer(allocator, _buffer, _allocation);
        _buffer     = VK_NULL_HANDLE;
        _allocation = VK_NULL_HANDLE;
    }

    _device        = nullptr;
    _mappedPointer = nullptr;
}

bool VulkanBuffer::createBuffer(
    vk::Buffer&                buffer,
    VmaAllocation&             allocation,
    const vk::DeviceSize       size,
    const vk::BufferUsageFlags usage,
    const VmaMemoryUsage       memoryUsage,
    const VulkanDevice*        device,
    std::string&               errorMessage
) {
    const VmaAllocator allocator = device->getAllocator();

    vk::BufferCreateInfo bufferInfo{};
    bufferInfo
        .setSize(size)
        .setUsage(usage)
        .setSharingMode(vk::SharingMode::eExclusive);

    VmaAllocationCreateInfo allocationInfo{};
    allocationInfo.usage = memoryUsage;

    VK_TRY(vmaCreateBuffer(allocator,
               reinterpret_cast<const VkBufferCreateInfo*>(&bufferInfo),
               &allocationInfo,
               reinterpret_cast<VkBuffer*>(&buffer),
               &allocation,
               nullptr),
        errorMessage);
    return true;
}

bool VulkanBuffer::copyBuffer(
    const vk::Buffer&           srcBuffer,
    const vk::Buffer&           dstBuffer,
    const vk::DeviceSize        size,
    const vk::DeviceSize        srcOffset,
    const vk::DeviceSize        dstOffset,
    const VulkanDevice*         device,
    const VulkanCommandManager* commandManager,
    std::string&                errorMessage
) {
    vk::CommandBuffer copyCommandBuffer;
    if (!commandManager->createCommandBuffer(copyCommandBuffer, errorMessage)) return false;

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
    const vk::Buffer&           srcBuffer,
    const VulkanCommandManager* commandManager,
    std::string&                errorMessage,
    vk::DeviceSize              size,
    const vk::DeviceSize        srcOffset,
    const vk::DeviceSize        dstOffset
) const {
    if (size == VK_WHOLE_SIZE) size = _bufferSize;

    if (!copyBuffer(srcBuffer, _buffer, size, srcOffset, dstOffset, _device, commandManager, errorMessage))
        return false;
    return true;
}

void* VulkanBuffer::mapMemory(std::string& errorMessage) {
    if (!_device || !_allocation) {
        errorMessage = "Failed to map buffer memory: device or memory not initialized";
        return nullptr;
    }

    const VmaAllocator allocator = _device->getAllocator();

    const auto memoryMap = VK_CALL(vmaMapMemory(allocator, _allocation, &_mappedPointer), errorMessage);
    if (memoryMap != VK_SUCCESS) {
        return nullptr;
    }
    return _mappedPointer;
}

void VulkanBuffer::unmapMemory() {
    if (_device && _allocation && _mappedPointer) {
        const VmaAllocator allocator = _device->getAllocator();
        vmaUnmapMemory(allocator, _allocation);
        _mappedPointer = nullptr;
    }
}
