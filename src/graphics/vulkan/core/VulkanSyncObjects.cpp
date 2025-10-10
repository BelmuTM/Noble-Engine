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

    imagesInFlight.resize(swapchainImageCount, VK_NULL_HANDLE);

    return true;
}

void VulkanSyncObjects::destroy() noexcept {
    if (!_device) return;

    cleanupOldSyncObjects();
    destroySyncObjects();
}

void VulkanSyncObjects::backup() {
    oldFences.insert(oldFences.end(), inFlightFences.begin(), inFlightFences.end());
    oldImageAvailable.insert(oldImageAvailable.end(), imageAvailableSemaphores.begin(), imageAvailableSemaphores.end());
    oldRenderFinished.insert(oldRenderFinished.end(), renderFinishedSemaphores.begin(), renderFinishedSemaphores.end());
}

bool VulkanSyncObjects::createSyncObjects(
    const uint32_t framesInFlight, const uint32_t swapchainImageCount, std::string& errorMessage
) {
    imageAvailableSemaphores.resize(framesInFlight);
    renderFinishedSemaphores.resize(swapchainImageCount);
    inFlightFences.resize(framesInFlight);

    constexpr vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};

    for (size_t i = 0; i < framesInFlight; i++) {
        VK_CREATE(_device.createSemaphore({}), imageAvailableSemaphores[i], errorMessage);
        VK_CREATE(_device.createFence(fenceInfo), inFlightFences[i], errorMessage);
    }

    for (auto& renderFinishedSemaphore : renderFinishedSemaphores) {
        VK_CREATE(_device.createSemaphore({}), renderFinishedSemaphore, errorMessage);
    }

    return true;
}

void VulkanSyncObjects::destroySyncObjects() {
    for (const auto& imageSemaphore : imageAvailableSemaphores) {
        _device.destroySemaphore(imageSemaphore);
    }

    for (const auto& renderSemaphore : renderFinishedSemaphores) {
        _device.destroySemaphore(renderSemaphore);
    }

    for (const auto& fence : inFlightFences) {
        _device.destroyFence(fence);
    }

    imageAvailableSemaphores.clear();
    renderFinishedSemaphores.clear();
    inFlightFences.clear();
}

void VulkanSyncObjects::cleanupOldSyncObjects() {
    for (const auto& imageSemaphore : oldImageAvailable) {
        _device.destroySemaphore(imageSemaphore);
    }

    for (const auto& renderSemaphore : oldRenderFinished) {
        _device.destroySemaphore(renderSemaphore);
    }

    for (const auto& fence : oldFences) {
        _device.destroyFence(fence);
    }

    oldImageAvailable.clear();
    oldRenderFinished.clear();
    oldFences.clear();
}
