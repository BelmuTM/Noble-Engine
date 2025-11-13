#include "VulkanSwapchainManager.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "core/debug/ErrorHandling.h"

#include <thread>

bool VulkanSwapchainManager::create(
    Platform::Window&    window,
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

std::optional<uint32_t> VulkanSwapchainManager::acquireNextImage(
    const uint32_t frameIndex, std::string& errorMessage, bool& discardLogging
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
        return std::nullopt;
    }

    const vk::Semaphore& imageAvailableSemaphore = _syncObjects.imageAvailableSemaphores[frameIndex];
    const vk::Fence&     inFlightFence           = _syncObjects.inFlightFences[frameIndex];

    while (vk::Result::eTimeout == logicalDevice.waitForFences(inFlightFence, vk::True, UINT64_MAX)) {
        std::this_thread::yield();
    }

    const auto nextImageAcquire =
        VK_CALL(logicalDevice.acquireNextImageKHR(_swapchain->handle(), UINT64_MAX, imageAvailableSemaphore, nullptr),
            errorMessage);

    if (nextImageAcquire.result == vk::Result::eErrorOutOfDateKHR ||
        nextImageAcquire.result == vk::Result::eSuboptimalKHR) {
        discardLogging = true;
        _outOfDate     = true;
        _window->setFramebufferResized(true);
        return std::nullopt;
    }

    if (nextImageAcquire.result != vk::Result::eSuccess) {
        return std::nullopt;
    }

    const uint32_t imageIndex = nextImageAcquire.value;

    if (imageIndex >= _swapchain->getImageCount()) {
        errorMessage =
            "Failed to record Vulkan command buffer: image index exceeds limit (" + std::to_string(imageIndex) + ")";
        return std::nullopt;
    }

    if (!waitForImageFence(frameIndex, imageIndex, errorMessage)) return std::nullopt;

    return imageIndex;
}

bool VulkanSwapchainManager::submitCommandBuffer(
    const VulkanCommandManager& commandManager,
    const uint32_t              frameIndex,
    const uint32_t              imageIndex,
    std::string&                errorMessage,
    bool&                       discardLogging
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

    const vk::CommandBuffer& currentBuffer = commandManager.getCommandBuffers()[frameIndex];

    const vk::Semaphore& imageAvailableSemaphore = _syncObjects.imageAvailableSemaphores[frameIndex];
    const vk::Fence&     inFlightFence           = _syncObjects.inFlightFences[frameIndex];
    const vk::Semaphore& renderFinishedSemaphore = _syncObjects.renderFinishedSemaphores[imageIndex];

    constexpr vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    vk::SubmitInfo submitInfo{};
    submitInfo
        .setWaitSemaphores(imageAvailableSemaphore)
        .setPWaitDstStageMask(&waitDestinationStageMask)
        .setCommandBuffers(currentBuffer)
        .setSignalSemaphores(renderFinishedSemaphore);

    VK_TRY(_device->getGraphicsQueue().submit(submitInfo, inFlightFence), errorMessage);

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
    const vk::Fence&  inFlightFence = _syncObjects.inFlightFences[frameIndex];

    if (const vk::Fence& imageInFlight = _syncObjects.imagesInFlight[imageIndex]) {
        VK_TRY(logicalDevice.waitForFences(imageInFlight, vk::True, UINT64_MAX), errorMessage);
    }
    _syncObjects.imagesInFlight[imageIndex] = inFlightFence;

    VK_TRY(logicalDevice.resetFences(inFlightFence), errorMessage);

    return true;
}
