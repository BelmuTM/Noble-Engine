#include "VulkanSwapchainManager.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

#include "core/debug/ErrorHandling.h"

#include <thread>

bool VulkanSwapchainManager::create(
    VulkanContext&    context,
    Platform::Window& window,
    const uint32_t    framesInFlight,
    const uint32_t    swapchainImageCount,
    std::string&      errorMessage
) noexcept {
    _context        = &context;
    _window         = &window;
    _framesInFlight = framesInFlight;

    TRY(_syncObjects.create(context.getDevice().getLogicalDevice(), framesInFlight, swapchainImageCount, errorMessage));
    return true;
}

void VulkanSwapchainManager::destroy() noexcept {
    _syncObjects.destroy();

    _context = nullptr;
    _window  = nullptr;
}

bool VulkanSwapchainManager::handleFramebufferResize(std::string& errorMessage) {
    if (!_window) return false;

    if (!_window->isFramebufferResized()) return true;

    TRY(recreateSwapchain(errorMessage));

    _window->setFramebufferResized(false);

    return true;
}

bool VulkanSwapchainManager::recreateSwapchain(std::string& errorMessage) {
    if (!_context) {
        errorMessage = "Failed to recreate Vulkan swapchain: context is null";
        return false;
    }

    VulkanSwapchain& swapchain = _context->getSwapchain();
    TRY(swapchain.recreate(_context->getSurface(), errorMessage));

    _syncObjects.backup();

    const vk::Device& logicalDevice       = _context->getDevice().getLogicalDevice();
    const uint32_t    swapchainImageCount = swapchain.getImages().size();

    TRY(_syncObjects.create(logicalDevice, _framesInFlight, swapchainImageCount, errorMessage));

    return true;
}

std::optional<uint32_t> VulkanSwapchainManager::acquireNextImage(
    const uint32_t frameIndex, std::string& errorMessage, bool& discardLogging
) {
    if (!_context) {
        errorMessage = "Failed to acquire Vulkan swapchain image: context is null";
        return false;
    }

    const vk::Device&      logicalDevice = _context->getDevice().getLogicalDevice();
    const VulkanSwapchain& swapchain     = _context->getSwapchain();

    if (swapchain.handle() == VK_NULL_HANDLE) {
        discardLogging = true;
        return std::nullopt;
    }

    const vk::Semaphore& imageAvailableSemaphore = _syncObjects.imageAvailableSemaphores[frameIndex];
    const vk::Fence&     inFlightFence           = _syncObjects.inFlightFences[frameIndex];

    while (vk::Result::eTimeout == logicalDevice.waitForFences(inFlightFence, vk::True, UINT64_MAX)) {
        std::this_thread::yield();
    }

    const auto nextImageAcquire =
        VK_CALL(logicalDevice.acquireNextImageKHR(swapchain.handle(), UINT64_MAX, imageAvailableSemaphore, nullptr),
            errorMessage);

    if (nextImageAcquire.result == vk::Result::eErrorOutOfDateKHR ||
        nextImageAcquire.result == vk::Result::eSuboptimalKHR) {
        discardLogging = true;
        if (!recreateSwapchain(errorMessage)) return std::nullopt;
        return std::nullopt;
    }

    if (nextImageAcquire.result != vk::Result::eSuccess) {
        return std::nullopt;
    }

    const uint32_t imageIndex = nextImageAcquire.value;

    if (imageIndex >= swapchain.getImages().size()) {
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
    if (!_context) {
        errorMessage = "Failed to submit Vulkan command buffer: context is null";
        return false;
    }

    const VulkanDevice&     device    = _context->getDevice();
    const vk::SwapchainKHR& swapchain = _context->getSwapchain().handle();

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

    VK_TRY(device.getGraphicsQueue().submit(submitInfo, inFlightFence), errorMessage);

    const vk::PresentInfoKHR presentInfo(renderFinishedSemaphore, swapchain, imageIndex);
    const auto queuePresent = VK_CALL(device.getPresentQueue().presentKHR(presentInfo), errorMessage);

    if (queuePresent == vk::Result::eErrorOutOfDateKHR || queuePresent == vk::Result::eSuboptimalKHR) {
        discardLogging = true;
        TRY(recreateSwapchain(errorMessage));
        _window->setFramebufferResized(false);
        return true;
    }
    if (queuePresent != vk::Result::eSuccess) return false;

    return true;
}

bool VulkanSwapchainManager::waitForImageFence(
    const uint32_t frameIndex, const uint32_t imageIndex, std::string& errorMessage
) {
    if (!_context) {
        errorMessage = "Failed to wait for Vulkan image fence: context is null";
        return false;
    }

    const vk::Device& logicalDevice = _context->getDevice().getLogicalDevice();
    const vk::Fence&  inFlightFence = _syncObjects.inFlightFences[frameIndex];

    if (const vk::Fence& imageInFlight = _syncObjects.imagesInFlight[imageIndex]) {
        VK_TRY(logicalDevice.waitForFences(imageInFlight, vk::True, UINT64_MAX), errorMessage);
    }
    _syncObjects.imagesInFlight[imageIndex] = inFlightFence;

    VK_TRY(logicalDevice.resetFences(inFlightFence), errorMessage);

    return true;
}
