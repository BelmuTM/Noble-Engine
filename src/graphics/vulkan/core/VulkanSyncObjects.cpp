#include "VulkanSyncObjects.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

bool VulkanSyncObjects::create(
    const vk::Device& device,
    const uint32_t    framesInFlight,
    const uint32_t    swapchainImageCount,
    std::string&      errorMessage
) noexcept {
    _device = device;

    if (!createSyncObjects(framesInFlight, swapchainImageCount, errorMessage)) return false;

    _imagesInFlight.resize(swapchainImageCount, VK_NULL_HANDLE);

    return true;
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

bool VulkanSyncObjects::createSyncObjects(
    const uint32_t framesInFlight, const uint32_t swapchainImageCount, std::string& errorMessage
) {
    _imageAvailableSemaphores.resize(framesInFlight);
    _renderFinishedSemaphores.resize(swapchainImageCount);
    _inFlightFences.resize(framesInFlight);

    constexpr vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};

    for (uint32_t i = 0; i < framesInFlight; i++) {
        VK_CREATE(_device.createSemaphore({}), _imageAvailableSemaphores[i], errorMessage);
        VK_CREATE(_device.createFence(fenceInfo), _inFlightFences[i], errorMessage);
    }

    for (auto& renderFinishedSemaphore : _renderFinishedSemaphores) {
        VK_CREATE(_device.createSemaphore({}), renderFinishedSemaphore, errorMessage);
    }

    return true;
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
