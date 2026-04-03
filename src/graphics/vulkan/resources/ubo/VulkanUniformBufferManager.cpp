#include "VulkanUniformBufferManager.h"

bool VulkanUniformBufferManager::create(
    const VulkanDevice& device, const std::uint32_t framesInFlight, std::string&
) noexcept {
    _device         = &device;
    _framesInFlight = framesInFlight;

    return true;
}

void VulkanUniformBufferManager::destroy() noexcept {
    for (const auto& uniformBuffer : _uniformBuffers) {
        uniformBuffer->destroy();
    }

    _uniformBuffers.clear();

    _device = nullptr;
}
