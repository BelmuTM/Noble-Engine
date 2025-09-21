#include "VulkanSyncObjects.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

bool VulkanSyncObjects::create(
    const vk::Device& device, const uint32_t framesInFlight, const uint32_t swapchainImageCount,
    std::string& errorMessage
) noexcept {
    _device = &device;

    if (!createSyncObjects(framesInFlight, swapchainImageCount, errorMessage)) return false;

    imagesInFlight.resize(swapchainImageCount, VK_NULL_HANDLE);
    return true;
}

void VulkanSyncObjects::destroy() noexcept {
    cleanupOldSyncObjects();
    destroySyncObjects();
    _device = nullptr;
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
        VK_CREATE(_device->createSemaphore({}), imageAvailableSemaphores[i], errorMessage);
        VK_CREATE(_device->createFence(fenceInfo), inFlightFences[i], errorMessage);
    }

    for (auto& renderFinishedSemaphore : renderFinishedSemaphores) {
        VK_CREATE(_device->createSemaphore({}), renderFinishedSemaphore, errorMessage);
    }
    return true;
}

void VulkanSyncObjects::destroySyncObjects() {
    for (const auto& imageSemaphore : imageAvailableSemaphores) {
        _device->destroySemaphore(imageSemaphore);
    }

    for (const auto& renderSemaphore : renderFinishedSemaphores) {
        _device->destroySemaphore(renderSemaphore);
    }

    for (const auto& fence : inFlightFences) {
        _device->destroyFence(fence);
    }

    imageAvailableSemaphores.clear();
    renderFinishedSemaphores.clear();
    inFlightFences.clear();
}

void VulkanSyncObjects::cleanupOldSyncObjects() {
    for (auto it = oldFences.begin(); it != oldFences.end();) {
        if (_device->getFenceStatus(*it) == vk::Result::eSuccess) {
            _device->destroyFence(*it);
            it = oldFences.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = oldImageAvailable.begin(); it != oldImageAvailable.end();) {
        _device->destroySemaphore(*it);
        it = oldImageAvailable.erase(it);
    }

    for (auto it = oldRenderFinished.begin(); it != oldRenderFinished.end();) {
        _device->destroySemaphore(*it);
        it = oldRenderFinished.erase(it);
    }
}
