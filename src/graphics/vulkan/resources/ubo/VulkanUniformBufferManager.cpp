#include "VulkanUniformBufferManager.h"

#include "core/debug/ErrorHandling.h"

bool VulkanUniformBufferManager::create(
    const VulkanDevice& device,
    const uint32_t      framesInFlight,
    std::string&        errorMessage
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

bool VulkanUniformBufferManager::createBuffer(
    VulkanUniformBufferBase& buffer, std::string& errorMessage
) {
    TRY(buffer.create(*_device, _framesInFlight, errorMessage));

    _uniformBuffers.push_back(&buffer);

    return true;
}
