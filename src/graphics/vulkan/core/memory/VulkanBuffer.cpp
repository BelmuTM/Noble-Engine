#include "VulkanBuffer.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "core/debug/ErrorHandling.h"

VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept {
    _device        = other._device;
    _buffer        = other._buffer;
    _bufferSize    = other._bufferSize;
    _deviceAddress = other._deviceAddress;
    _allocation    = other._allocation;
    _mappedPointer = other._mappedPointer;

    other._device        = nullptr;
    other._buffer        = VK_NULL_HANDLE;
    other._bufferSize    = 0;
    other._deviceAddress = 0;
    other._allocation    = VK_NULL_HANDLE;
    other._mappedPointer = VK_NULL_HANDLE;
}

VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& other) noexcept {
    if (this != &other) {
        destroy();

        _device        = other._device;
        _buffer        = other._buffer;
        _bufferSize    = other._bufferSize;
        _deviceAddress = other._deviceAddress;
        _allocation    = other._allocation;
        _mappedPointer = other._mappedPointer;

        other._device        = nullptr;
        other._buffer        = VK_NULL_HANDLE;
        other._bufferSize    = 0;
        other._deviceAddress = 0;
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

    TRY(createBuffer(_buffer, _allocation, size, usage, memoryUsage, device, errorMessage));

    _bufferSize = size;

    // Check if the buffer can be used to get its device address
    if (usage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
        vk::BufferDeviceAddressInfo deviceAddressInfo{};
        deviceAddressInfo.setBuffer(_buffer);
        _deviceAddress = _device->getLogicalDevice().getBufferAddress(deviceAddressInfo);
    }

    return true;
}

void VulkanBuffer::destroy() noexcept {
    if (!_device) return;

    const VmaAllocator& allocator = _device->getAllocator();

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
    if (size == 0) {
        errorMessage = "Failed to create Vulkan buffer: size is 0";
        return false;
    }

    const VmaAllocator& allocator = device->getAllocator();

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
        errorMessage
    );

    return true;
}

bool VulkanBuffer::copyBuffer(
    const vk::Buffer&           srcBuffer,
    const vk::Buffer&           dstBuffer,
    const vk::DeviceSize        size,
    const vk::DeviceSize        srcOffset,
    const vk::DeviceSize        dstOffset,
    const VulkanCommandManager* commandManager,
    std::string&                errorMessage
) {
    vk::CommandBuffer copyCommandBuffer{};
    TRY(commandManager->beginSingleTimeCommands(copyCommandBuffer, errorMessage));

    vk::BufferCopy2 copyRegion{srcOffset, dstOffset, size};

    vk::CopyBufferInfo2 copyBufferInfo{};
    copyBufferInfo
        .setSrcBuffer(srcBuffer)
        .setDstBuffer(dstBuffer)
        .setRegions({copyRegion});

    copyCommandBuffer.copyBuffer2(copyBufferInfo);

    TRY(commandManager->endSingleTimeCommands(copyCommandBuffer, errorMessage));

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

    TRY(copyBuffer(srcBuffer, _buffer, size, srcOffset, dstOffset, commandManager, errorMessage));

    return true;
}

void* VulkanBuffer::mapMemory(std::string& errorMessage) {
    if (!_device || !_allocation) {
        errorMessage = "Failed to map buffer memory: device or memory not initialized";
        return nullptr;
    }

    const VmaAllocator& allocator = _device->getAllocator();

    const auto memoryMap = VK_CALL(vmaMapMemory(allocator, _allocation, &_mappedPointer), errorMessage);
    if (memoryMap != VK_SUCCESS) {
        return nullptr;
    }

    return _mappedPointer;
}

void VulkanBuffer::unmapMemory() {
    if (_device && _allocation && _mappedPointer) {
        const VmaAllocator& allocator = _device->getAllocator();
        vmaUnmapMemory(allocator, _allocation);
        _mappedPointer = nullptr;
    }
}
