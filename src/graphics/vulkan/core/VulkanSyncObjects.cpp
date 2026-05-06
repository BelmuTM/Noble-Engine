#include "VulkanSyncObjects.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

Expected<void> VulkanSyncObjects::create(
    const vk::Device&   device,
    const std::uint32_t framesInFlight,
    const std::uint32_t swapchainImageCount
) noexcept {
    _device = device;

    TRY(createSyncObjects(framesInFlight, swapchainImageCount));

    _imagesInFlight.resize(swapchainImageCount, VK_NULL_HANDLE);

    return {};
}

void VulkanSyncObjects::destroy() noexcept {
    if (!_device) return;

    cleanupOldSyncObjects();
    destroySyncObjects();
}

void VulkanSyncObjects::backup() {
    _oldFences.insert(_oldFences.end(), _inFlightFences.begin(), _inFlightFences.end());
    _oldImageAvailable.insert(_oldImageAvailable.end(), _imageAvailableSemaphores.begin(), _imageAvailableSemaphores.end());
    _oldRenderFinished.insert(_oldRenderFinished.end(), _renderFinishedSemaphores.begin(), _renderFinishedSemaphores.end());
}

Expected<void> VulkanSyncObjects::createSyncObjects(
    const std::uint32_t framesInFlight, const std::uint32_t swapchainImageCount
) {
    _imageAvailableSemaphores.resize(framesInFlight);
    _renderFinishedSemaphores.resize(swapchainImageCount);
    _inFlightFences.resize(framesInFlight);

    constexpr vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};

    for (std::uint32_t i = 0; i < framesInFlight; i++) {
        VK_CREATE(_imageAvailableSemaphores[i], _device.createSemaphore({}));
        VK_CREATE(_inFlightFences[i], _device.createFence(fenceInfo));
    }

    for (auto& renderFinishedSemaphore : _renderFinishedSemaphores) {
        VK_CREATE(renderFinishedSemaphore, _device.createSemaphore({}));
    }

    return {};
}

void VulkanSyncObjects::destroySyncObjects() {
    for (const auto& imageSemaphore : _imageAvailableSemaphores) {
        _device.destroySemaphore(imageSemaphore);
    }

    for (const auto& renderSemaphore : _renderFinishedSemaphores) {
        _device.destroySemaphore(renderSemaphore);
    }

    for (const auto& fence : _inFlightFences) {
        _device.destroyFence(fence);
    }

    _imageAvailableSemaphores.clear();
    _renderFinishedSemaphores.clear();
    _inFlightFences.clear();
}

void VulkanSyncObjects::cleanupOldSyncObjects() {
    for (const auto& imageSemaphore : _oldImageAvailable) {
        _device.destroySemaphore(imageSemaphore);
    }

    for (const auto& renderSemaphore : _oldRenderFinished) {
        _device.destroySemaphore(renderSemaphore);
    }

    for (const auto& fence : _oldFences) {
        _device.destroyFence(fence);
    }

    _oldImageAvailable.clear();
    _oldRenderFinished.clear();
    _oldFences.clear();
}
