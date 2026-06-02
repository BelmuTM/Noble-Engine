#include "VulkanStorageBuffer.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

Expected<void> VulkanStorageBuffer::create(
    const VulkanDevice&  device,
    const std::uint32_t  framesInFlight,
    const vk::DeviceSize size
) noexcept {
    _device         = &device;
    _framesInFlight = framesInFlight;
    _bufferSize     = size;

    TRY(createStorageBuffers());

    return {};
}

void VulkanStorageBuffer::destroy() noexcept {
    for (auto& storageBuffer : _storageBuffers) {
        storageBuffer.destroy();
    }

    _storageBuffers.clear();

    _device = nullptr;
}

Expected<void> VulkanStorageBuffer::createStorageBuffers() {
    if (!_device) {
        return VK_FAIL("Failed to create storage buffers: device is null.");
    }

    _storageBuffers.clear();
    _storageBuffers.reserve(_framesInFlight);

    for (std::uint32_t i = 0; i < _framesInFlight; i++) {
        VulkanBuffer storageBuffer;

        TRY(storageBuffer.create(
            getBufferSize(),
            vk::BufferUsageFlagBits::eStorageBuffer,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            _device
        ));

        TRY(storageBuffer.mapMemory());

        _storageBuffers.emplace_back(std::move(storageBuffer));
    }

    return {};
}
