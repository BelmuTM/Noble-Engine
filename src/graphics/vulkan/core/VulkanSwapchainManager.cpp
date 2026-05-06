#include "VulkanSwapchainManager.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include <thread>

Expected<void> VulkanSwapchainManager::create(
    Window&              window,
    const VulkanSurface& surface,
    const VulkanDevice&  device,
    VulkanSwapchain&     swapchain,
    const std::uint32_t  framesInFlight
) noexcept {
    _window         = &window;
    _surface        = &surface;
    _device         = &device;
    _swapchain      = &swapchain;
    _framesInFlight = framesInFlight;

    TRY(_syncObjects.create(device.getLogicalDevice(), framesInFlight, swapchain.getImageCount()));

    return {};
}

void VulkanSwapchainManager::destroy() noexcept {
    _syncObjects.destroy();

    _window    = nullptr;
    _surface   = nullptr;
    _device    = nullptr;
    _swapchain = nullptr;
}

Expected<void> VulkanSwapchainManager::recreateSwapchain() {
    if (!_swapchain) {
        return VK_FAIL("Failed to recreate swapchain: swapchain is null.");
    }

    TRY(_swapchain->recreate(_surface->handle()));

    _syncObjects.backup();

    const vk::Device&   logicalDevice       = _device->getLogicalDevice();
    const std::uint32_t swapchainImageCount = _swapchain->getImageCount();

    TRY(_syncObjects.create(logicalDevice, _framesInFlight, swapchainImageCount));

    return {};
}

Expected<VulkanSwapchain::SwapchainOp<uint32_t>> VulkanSwapchainManager::acquireNextImage(
    const std::uint32_t frameIndex) {
    if (!_device) {
        return VK_FAIL("Failed to acquire swapchain image: device is null.");
    }

    if (!_swapchain) {
        return VK_FAIL("Failed to acquire swapchain image: swapchain is null.");
    }

    if (_swapchain->handle() == VK_NULL_HANDLE) {
        return VK_FAIL("Failed to acquire swapchain image: swapchain handle is null.");
    }

    const vk::Device& logicalDevice = _device->getLogicalDevice();

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

    Failure acquireFailure;

    const auto imageAcquire = VK_RESULT(logicalDevice.acquireNextImage2KHR(acquireNextImageInfo), &acquireFailure);

    const uint32_t imageIndex = imageAcquire.value;

    switch (imageAcquire.result) {
        case vk::Result::eErrorOutOfDateKHR:
            _window->setFramebufferResized(true);
            return Expected<VulkanSwapchain::SwapchainOp<uint32_t>>({VulkanSwapchain::Status::OutOfDate, std::nullopt});

        case vk::Result::eSuboptimalKHR:
            _window->setFramebufferResized(true);
            return Expected<VulkanSwapchain::SwapchainOp<uint32_t>>({VulkanSwapchain::Status::Suboptimal, std::nullopt});

        case vk::Result::eSuccess:
            break;

        default:
            return Unexpected(acquireFailure);
    }

    assert(imageIndex < _swapchain->getImageCount());

    TRY(waitForImageFence(frameIndex, imageIndex));

    return Expected<VulkanSwapchain::SwapchainOp<uint32_t>>({VulkanSwapchain::Status::Success, imageIndex});
}

Expected<VulkanSwapchain::SwapchainOpVoid> VulkanSwapchainManager::submitCommandBuffer(
    const vk::CommandBuffer commandBuffer,
    const std::uint32_t     frameIndex,
    const std::uint32_t     imageIndex
) const {
    if (!_device) {
        return VK_FAIL("Failed to submit command buffer: device is null.");
    }

    if (!_swapchain) {
        return VK_FAIL("Failed to submit command buffer: swapchain is null.");
    }

    if (_swapchain->handle() == VK_NULL_HANDLE) {
        return VK_FAIL("Failed to submit command buffer: swapchain handle is null.");
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

    VK_TRY(_device->getLogicalDevice().resetFences(1, &inFlightFence));

    VK_TRY(_device->getGraphicsQueue().submit2(submitInfo, inFlightFence));

    const vk::PresentInfoKHR presentInfo(renderFinishedSemaphore, _swapchain->handle(), imageIndex);

    Failure presentFailure;

    const auto queuePresent = VK_RESULT(_device->getPresentQueue().presentKHR(presentInfo), &presentFailure);

    switch (queuePresent) {
        case vk::Result::eErrorOutOfDateKHR: {
            _window->setFramebufferResized(true);
            return Expected<VulkanSwapchain::SwapchainOpVoid>({VulkanSwapchain::Status::OutOfDate});
        }

        case vk::Result::eSuboptimalKHR:
            return Expected<VulkanSwapchain::SwapchainOpVoid>({VulkanSwapchain::Status::Suboptimal});

        case vk::Result::eSuccess:
            break;

        default:
            return Unexpected(presentFailure);
    }

    return Expected<VulkanSwapchain::SwapchainOpVoid>({VulkanSwapchain::Status::Success});
}

Expected<void> VulkanSwapchainManager::waitForImageFence(
    const std::uint32_t frameIndex, const std::uint32_t imageIndex
) {
    if (!_device) {
        return VK_FAIL("Failed to wait for image fence: device is null.");
    }

    const vk::Device& logicalDevice = _device->getLogicalDevice();
    const vk::Fence&  inFlightFence = _syncObjects.getInFlightFence(frameIndex);

    if (const vk::Fence& imageInFlight = _syncObjects.getImagesInFlightFence(imageIndex)) {
        VK_TRY(logicalDevice.waitForFences(imageInFlight, vk::True, UINT64_MAX));
    }

    _syncObjects.getImagesInFlightFences()[imageIndex] = inFlightFence;

    VK_TRY(logicalDevice.resetFences(inFlightFence));

    return {};
}
