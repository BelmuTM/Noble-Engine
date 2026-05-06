#include "VulkanStorageBufferManager.h"

Expected<void> VulkanStorageBufferManager::create(
    const VulkanDevice& device, const std::uint32_t framesInFlight
) noexcept {
    _device         = &device;
    _framesInFlight = framesInFlight;

    return {};
}

void VulkanStorageBufferManager::destroy() noexcept {
    for (const auto& storageBuffer : _storageBuffers) {
        storageBuffer->destroy();
    }

    _storageBuffers.clear();

    _device = nullptr;
}

Expected<VulkanStorageBuffer*> VulkanStorageBufferManager::allocateBuffer(const vk::DeviceSize size) {
    _storageBuffers.push_back(std::make_unique<VulkanStorageBuffer>());

    TRY(_storageBuffers.back()->create(*_device, _framesInFlight, size));

    return Expected(_storageBuffers.back().get());
}
