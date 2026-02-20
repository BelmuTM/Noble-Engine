#include "VulkanObjectBuffer.h"

#include "core/debug/ErrorHandling.h"

bool VulkanObjectBuffer::create(
    const VulkanDevice& device, const uint32_t maxObjects, std::string& errorMessage
) noexcept {
    _device     = &device;
    _maxObjects = maxObjects;

    const vk::DeviceSize objectBufferSize = sizeof(ObjectDataGPU) * maxObjects;

    TRY_deprecated(VulkanBuffer::create(
        objectBufferSize,
        vk::BufferUsageFlagBits::eStorageBuffer |
        vk::BufferUsageFlagBits::eShaderDeviceAddress,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        _device,
        errorMessage
    ));

    TRY_deprecated(mapMemory(errorMessage));

    return true;
}

void VulkanObjectBuffer::destroy() noexcept {
    VulkanBuffer::destroy();

    _device = nullptr;
}

void VulkanObjectBuffer::update(const uint32_t objectIndex, const ObjectDataGPU& dataToGPU) const {
#if defined(VULKAN_DEBUG_UTILS)
    assert(objectIndex < _maxObjects);
#endif

    auto* bufferPointer = static_cast<ObjectDataGPU*>(getMappedPointer());
    bufferPointer[objectIndex] = dataToGPU;
}

void VulkanObjectBuffer::update(const std::vector<ObjectDataGPU>& dataToGPU) const {
    std::memcpy(getMappedPointer(), dataToGPU.data(), dataToGPU.size() * sizeof(ObjectDataGPU));
}
