#include "VulkanStorageBuffer.h"

#include "core/debug/ErrorHandling.h"

bool VulkanStorageBuffer::create(
    const VulkanDevice&  device,
    const std::uint32_t  framesInFlight,
    const vk::DeviceSize size,
    std::string&         errorMessage
) noexcept {
    _device         = &device;
    _framesInFlight = framesInFlight;
    _bufferSize     = size;

    TRY_BOOL(createStorageBuffers(errorMessage));

    return true;
}

void VulkanStorageBuffer::destroy() noexcept {
    for (auto& storageBuffer : _storageBuffers) {
        storageBuffer.destroy();
    }

    _storageBuffers.clear();

    _device = nullptr;
}

bool VulkanStorageBuffer::createStorageBuffers(std::string& errorMessage) {
    if (!_device) {
        errorMessage = "Failed to create Vulkan storage buffers: device is null.";
        return false;
    }

    _storageBuffers.clear();
    _storageBuffers.reserve(_framesInFlight);

    for (std::uint32_t i = 0; i < _framesInFlight; i++) {
        VulkanBuffer storageBuffer;

        // TODO: Add the possibility to decide memory usage.
        TRY_BOOL(storageBuffer.create(
            getBufferSize(),
            vk::BufferUsageFlagBits::eStorageBuffer,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            _device,
            errorMessage
        ));

        TRY_BOOL(storageBuffer.mapMemory(errorMessage));

        _storageBuffers.emplace_back(std::move(storageBuffer));
    }

    return true;
}
