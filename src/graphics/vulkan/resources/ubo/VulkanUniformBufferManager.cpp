#include "VulkanUniformBufferManager.h"

Expected<void> VulkanUniformBufferManager::create(
    const VulkanDevice& device, const std::uint32_t framesInFlight
) noexcept {
    _device         = &device;
    _framesInFlight = framesInFlight;

    return {};
}

void VulkanUniformBufferManager::destroy() noexcept {
    for (const auto& uniformBuffer : _uniformBuffers) {
        uniformBuffer->destroy();
    }

    _uniformBuffers.clear();

    _device = nullptr;
}
