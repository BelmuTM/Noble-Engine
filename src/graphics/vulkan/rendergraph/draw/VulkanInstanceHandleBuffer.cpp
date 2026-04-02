#include "VulkanInstanceHandleBuffer.h"

#include "core/debug/ErrorHandling.h"

#include <cstring>

bool VulkanInstanceHandleBuffer::create(
    const VulkanDevice& device, const std::uint32_t maxInstances, std::string& errorMessage
) noexcept {
    _device       = &device;
    _maxInstances = maxInstances;

    const vk::DeviceSize instanceBufferSize = maxInstances * sizeof(VulkanInstanceHandle);

    TRY_BOOL(VulkanBuffer::create(
        instanceBufferSize,
        vk::BufferUsageFlagBits::eStorageBuffer |
        vk::BufferUsageFlagBits::eShaderDeviceAddress,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        _device,
        errorMessage
    ));

    TRY_BOOL(mapMemory(errorMessage));

    return true;
}

void VulkanInstanceHandleBuffer::destroy() noexcept {
    VulkanBuffer::destroy();

    _device = nullptr;
}

void VulkanInstanceHandleBuffer::update(const std::vector<VulkanInstanceHandle>& handles) const {
#if defined(VULKAN_DEBUG_UTILS)
    assert(handles.size() <= _maxInstances);
#endif

    std::memcpy(getMappedPointer(), handles.data(), handles.size() * sizeof(VulkanInstanceHandle));
}
