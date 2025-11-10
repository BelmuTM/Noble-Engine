#include "VulkanObjectBuffer.h"

#include "core/debug/ErrorHandling.h"

bool VulkanObjectBuffer::create(
    const VulkanDevice& device, const uint32_t maxObjects, std::string& errorMessage
) noexcept {
    _device     = &device;
    _maxObjects = maxObjects;

    const vk::DeviceSize objectBufferSize = sizeof(ObjectDataGPU) * maxObjects;

    TRY(_buffer.create(
        objectBufferSize,
        vk::BufferUsageFlagBits::eStorageBuffer |
        vk::BufferUsageFlagBits::eShaderDeviceAddress,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        _device,
        errorMessage
    ));

    TRY(_buffer.mapMemory(errorMessage));

    return true;
}

void VulkanObjectBuffer::destroy() noexcept {
    _buffer.destroy();

    _device = nullptr;
}

void VulkanObjectBuffer::update(const uint32_t objectIndex, const ObjectDataGPU& data) const {
#if VULKAN_DEBUG_UTILS
    assert(objectIndex < _maxObjects);
#endif

    auto* bufferPointer = static_cast<ObjectDataGPU*>(_buffer.getMappedPointer());
    bufferPointer[objectIndex] = data;
}
