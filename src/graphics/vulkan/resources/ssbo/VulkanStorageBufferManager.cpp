#include "VulkanStorageBufferManager.h"

#include "core/debug/ErrorHandling.h"

bool VulkanStorageBufferManager::create(
    const VulkanDevice& device, const std::uint32_t framesInFlight, std::string&
) noexcept {
    _device         = &device;
    _framesInFlight = framesInFlight;

    return true;
}

void VulkanStorageBufferManager::destroy() noexcept {
    for (const auto& storageBuffer : _storageBuffers) {
        storageBuffer->destroy();
    }

    _storageBuffers.clear();

    _device = nullptr;
}

VulkanStorageBuffer* VulkanStorageBufferManager::allocateBuffer(const vk::DeviceSize size, std::string& errorMessage) {
    _storageBuffers.push_back(std::make_unique<VulkanStorageBuffer>());

    if (!_storageBuffers.back()->create(*_device, _framesInFlight, size, errorMessage)) {
        return nullptr;
    }

    return _storageBuffers.back().get();
}
