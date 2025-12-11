#include "VulkanSwapchainManager.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "core/debug/ErrorHandling.h"

#include <thread>

bool VulkanSwapchainManager::create(
    Window&              window,
    const VulkanSurface& surface,
    const VulkanDevice&  device,
    VulkanSwapchain&     swapchain,
    const uint32_t       framesInFlight,
    std::string&         errorMessage
) noexcept {
    _window         = &window;
    _surface        = &surface;
    _device         = &device;
    _swapchain      = &swapchain;
    _framesInFlight = framesInFlight;

    TRY(_syncObjects.create(device.getLogicalDevice(), framesInFlight, swapchain.getImageCount(), errorMessage));

    return true;
}

void VulkanSwapchainManager::destroy() noexcept {
    _syncObjects.destroy();

    _window    = nullptr;
    _surface   = nullptr;
    _device    = nullptr;
    _swapchain = nullptr;
}

bool VulkanSwapchainManager::recreateSwapchain(std::string& errorMessage) {
    if (!_swapchain) {
        errorMessage = "Failed to recreate Vulkan swapchain: swapchain is null";
        return false;
    }

    TRY(_swapchain->recreate(*_surface, errorMessage));

    _syncObjects.backup();

    const vk::Device& logicalDevice       = _device->getLogicalDevice();
    const uint32_t    swapchainImageCount = _swapchain->getImageCount();

    TRY(_syncObjects.create(logicalDevice, _framesInFlight, swapchainImageCount, errorMessage));

    return true;
}

bool VulkanSwapchainManager::acquireNextImage(
    uint32_t& imageIndex, const uint32_t frameIndex, std::string& errorMessage, bool& discardLogging
) {
    if (!_device) {
        errorMessage = "Failed to acquire Vulkan swapchain image: device is null";
        return false;
    }

    if (!_swapchain) {
        errorMessage = "Failed to acquire Vulkan swapchain image: swapchain is null";
        return false;
    }

    const vk::Device& logicalDevice = _device->getLogicalDevice();

    if (_swapchain->handle() == VK_NULL_HANDLE) {
        discardLogging = true;
        return false;
    }

    const vk::Semaphore& imageAvailableSemaphore = _syncObjects.getImageAvailableSemaphore(frameIndex);
    const vk::Fence&     inFlightFence           = _syncObjects.getInFlightFence(frameIndex);

    while (vk::Result::eTimeout == logicalDevice.waitForFences(inFlightFence, vk::True, UINT64_MAX)) {
        std::this_thread::yield();
    }

    vk::AcquireNextImageInfoKHR acquireNextImageInfo{};
    acquireNextImageInfo
        .setSwapchain(_swapchain->handle())
        .setTimeout(UINT64_MAX)
        .setSemaphore(imageAvailableSemaphore)
        .setFence(nullptr)
        .setDeviceMask(1);

    const auto imageAcquire = VK_CALL(logicalDevice.acquireNextImage2KHR(acquireNextImageInfo), errorMessage);

    if (imageAcquire.result == vk::Result::eErrorOutOfDateKHR ||
        imageAcquire.result == vk::Result::eSuboptimalKHR) {
        discardLogging = true;
        _outOfDate     = true;
        _window->setFramebufferResized(true);
        return false;
    }

    if (imageAcquire.result != vk::Result::eSuccess) {
        return false;
    }

    imageIndex = imageAcquire.value;

    if (imageIndex >= _swapchain->getImageCount()) {
        errorMessage =
            "Failed to acquire Vulkan image: image index exceeds limit (" + std::to_string(imageIndex) + ")";
        return false;
    }

    if (!waitForImageFence(frameIndex, imageIndex, errorMessage)) return false;

    return true;
}

bool VulkanSwapchainManager::submitCommandBuffer(
    const vk::CommandBuffer commandBuffer,
    const uint32_t          frameIndex,
    const uint32_t          imageIndex,
    std::string&            errorMessage,
    bool&                   discardLogging
) {
    if (!_device) {
        errorMessage = "Failed to submit Vulkan command buffer: device is null";
        return false;
    }

    if (!_swapchain) {
        errorMessage = "Failed to submit Vulkan command buffer: swapchain is null";
        return false;
    }

    const vk::SwapchainKHR& swapchain = _swapchain->handle();

    if (swapchain == VK_NULL_HANDLE) {
        discardLogging = true;
        return false;
    }

    const vk::Semaphore& imageAvailableSemaphore = _syncObjects.getImageAvailableSemaphore(frameIndex);
    const vk::Fence&     inFlightFence           = _syncObjects.getInFlightFence(frameIndex);
    const vk::Semaphore& renderFinishedSemaphore = _syncObjects.getRenderFinishedSemaphore(imageIndex);

    constexpr vk::PipelineStageFlags2 waitDestinationStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput);

    vk::SemaphoreSubmitInfo imageAvailableSemaphoreInfo{};
    imageAvailableSemaphoreInfo
        .setSemaphore(imageAvailableSemaphore)
        .setStageMask(waitDestinationStageMask);

    vk::SemaphoreSubmitInfo renderFinishedSemaphoreInfo{};
    renderFinishedSemaphoreInfo
        .setSemaphore(renderFinishedSemaphore)
        .setStageMask(waitDestinationStageMask);

    vk::CommandBufferSubmitInfo commandBufferInfo{};
    commandBufferInfo.setCommandBuffer(commandBuffer);

    vk::SubmitInfo2 submitInfo{};
    submitInfo
        .setWaitSemaphoreInfos(imageAvailableSemaphoreInfo)
        .setCommandBufferInfos(commandBufferInfo)
        .setSignalSemaphoreInfos(renderFinishedSemaphoreInfo);

    VK_TRY(_device->getLogicalDevice().resetFences(1, &inFlightFence), errorMessage);

    VK_TRY(_device->getGraphicsQueue().submit2(submitInfo, inFlightFence), errorMessage);

    const vk::PresentInfoKHR presentInfo(renderFinishedSemaphore, swapchain, imageIndex);

    const auto queuePresent = VK_CALL(_device->getPresentQueue().presentKHR(presentInfo), errorMessage);

    if (queuePresent == vk::Result::eErrorOutOfDateKHR ||
        queuePresent == vk::Result::eSuboptimalKHR) {
        discardLogging = true;
        _outOfDate     = true;
        _window->setFramebufferResized(true);
        return true;
    }
    if (queuePresent != vk::Result::eSuccess) return false;

    _outOfDate = false;

    return true;
}

bool VulkanSwapchainManager::waitForImageFence(
    const uint32_t frameIndex, const uint32_t imageIndex, std::string& errorMessage
) {
    if (!_device) {
        errorMessage = "Failed to wait for Vulkan image fence: device is null";
        return false;
    }

    const vk::Device& logicalDevice = _device->getLogicalDevice();
    const vk::Fence&  inFlightFence = _syncObjects.getInFlightFence(frameIndex);

    if (const vk::Fence& imageInFlight = _syncObjects.getImagesInFlightFence(imageIndex)) {
        VK_TRY(logicalDevice.waitForFences(imageInFlight, vk::True, UINT64_MAX), errorMessage);
    }

    _syncObjects.getImagesInFlightFences()[imageIndex] = inFlightFence;

    VK_TRY(logicalDevice.resetFences(inFlightFence), errorMessage);

    return true;
}
