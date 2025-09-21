#include "VulkanRenderer.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

#include "core/Engine.h"
#include "core/debug/Logger.h"

#include <iostream>

static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

VulkanRenderer::~VulkanRenderer() {
    shutdown();
}

bool VulkanRenderer::init(Platform::Window& window) {
    _window = &window;

    std::string errorMessage = "Failed to create Vulkan context: no error message provided";

    const auto rollbackAndExit = [&](void*) {
        shutdown();
        Engine::fatalExit(errorMessage);
    };
    std::unique_ptr<void, decltype(rollbackAndExit)> guard(nullptr, rollbackAndExit);

    if (!createVulkanEntity(&context, errorMessage, window)) return false;

    const VulkanDevice&    device    = context.getDevice();
    const VulkanSwapchain& swapchain = context.getSwapchain();

    if (!createVulkanEntity(&commandManager, errorMessage, device, swapchain, MAX_FRAMES_IN_FLIGHT)) return false;

    const vk::Device& logicalDevice       = device.getLogicalDevice();
    const uint32_t    swapchainImageCount = swapchain.getImages().size();

    if (!createVulkanEntity(&syncObjects, errorMessage, logicalDevice, MAX_FRAMES_IN_FLIGHT, swapchainImageCount))
        return false;

    if (!createVulkanEntity(&graphicsPipeline, errorMessage, logicalDevice, swapchain)) return false;

    guard.release();
    return true;
}

void VulkanRenderer::shutdown() {
   VK_CALL_LOG(context.getDevice().getLogicalDevice().waitIdle(), Logger::Level::ERROR);
}

void VulkanRenderer::drawFrame() {
    std::string errorMessage;

    if (!handleFramebufferResize(errorMessage)) return;

    const auto imageIndexOpt = acquireNextImage(errorMessage);
    if (!imageIndexOpt) return;

    const uint32_t imageIndex = *imageIndexOpt;

    waitForImageFence(imageIndex);
    recordCurrentCommandBuffer(imageIndex);

    if (!submitCommandBuffer(imageIndex, errorMessage)) return;

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

bool VulkanRenderer::handleFramebufferResize(std::string& errorMessage) {
    if (!_window->isFramebufferResized()) return true;

    if (!recreateSwapchain(errorMessage)) {
        Logger::error(errorMessage);
        return false;
    }
    _window->setFramebufferResized(false);
    return true;
}

std::optional<uint32_t> VulkanRenderer::acquireNextImage(std::string& errorMessage) {
    const vk::Device&      logicalDevice = context.getDevice().getLogicalDevice();
    const VulkanSwapchain& swapchain     = context.getSwapchain();

    const vk::Semaphore& imageAvailableSemaphore = syncObjects.imageAvailableSemaphores[currentFrame];
    const vk::Fence&     inFlightFence           = syncObjects.inFlightFences[currentFrame];

    while (vk::Result::eTimeout == logicalDevice.waitForFences(inFlightFence, vk::True, UINT64_MAX)) {}

    const auto nextImageAcquire =
        VK_CALL(logicalDevice.acquireNextImageKHR(swapchain.handle(), UINT64_MAX, imageAvailableSemaphore, nullptr),
                errorMessage);

    if (nextImageAcquire.result == vk::Result::eErrorOutOfDateKHR ||
        nextImageAcquire.result == vk::Result::eSuboptimalKHR) {
        recreateSwapchain(errorMessage);
        return std::nullopt;
    }

    if (nextImageAcquire.result != vk::Result::eSuccess) {
        Logger::error(errorMessage);
        return std::nullopt;
    }

    const uint32_t imageIndex = nextImageAcquire.value;

    if (imageIndex >= swapchain.getImages().size()) {
        errorMessage =
            "Failed to record Vulkan command buffer: image index exceeds limit (" + std::to_string(imageIndex) + ")";
        Logger::error(errorMessage);
        return std::nullopt;
    }
    return imageIndex;
}

void VulkanRenderer::waitForImageFence(const uint32_t imageIndex) {
    const vk::Device& logicalDevice = context.getDevice().getLogicalDevice();
    const vk::Fence&  inFlightFence = syncObjects.inFlightFences[currentFrame];

    if (const vk::Fence imageInFlight = syncObjects.imagesInFlight[imageIndex]) {
        VK_CALL_LOG(logicalDevice.waitForFences(imageInFlight, vk::True, UINT64_MAX), Logger::Level::ERROR);
    }
    syncObjects.imagesInFlight[imageIndex] = inFlightFence;

    VK_TRY_VOID_LOG(logicalDevice.resetFences(inFlightFence), Logger::Level::ERROR);
}

void VulkanRenderer::recordCurrentCommandBuffer(const uint32_t imageIndex) {
    const vk::CommandBuffer& currentBuffer = commandManager.getCommandBuffers()[currentFrame];
    VK_CALL_LOG(currentBuffer.reset(), Logger::Level::ERROR);

    commandManager.recordCommandBuffer(currentBuffer, imageIndex, graphicsPipeline);
}

bool VulkanRenderer::submitCommandBuffer(const uint32_t imageIndex, std::string& errorMessage) {
    const VulkanDevice&     device    = context.getDevice();
    const vk::SwapchainKHR& swapchain = context.getSwapchain().handle();

    const vk::CommandBuffer& currentBuffer = commandManager.getCommandBuffers()[currentFrame];

    const vk::Semaphore& imageAvailableSemaphore = syncObjects.imageAvailableSemaphores[currentFrame];
    const vk::Fence&     inFlightFence           = syncObjects.inFlightFences[currentFrame];
    const vk::Semaphore& renderFinishedSemaphore = syncObjects.renderFinishedSemaphores[imageIndex];

    constexpr vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    vk::SubmitInfo submitInfo{};
    submitInfo
        .setWaitSemaphoreCount(1)
        .setPWaitSemaphores(&imageAvailableSemaphore)
        .setPWaitDstStageMask(&waitDestinationStageMask)
        .setCommandBufferCount(1)
        .setPCommandBuffers(&currentBuffer)
        .setSignalSemaphoreCount(1)
        .setPSignalSemaphores(&renderFinishedSemaphore);

    VK_TRY_LOG(device.getGraphicsQueue().submit(submitInfo, inFlightFence), Logger::Level::ERROR);

    const vk::PresentInfoKHR presentInfo(renderFinishedSemaphore, swapchain, imageIndex);
    const auto queuePresent = VK_CALL(device.getPresentQueue().presentKHR(presentInfo), errorMessage);

    if (queuePresent == vk::Result::eErrorOutOfDateKHR || queuePresent == vk::Result::eSuboptimalKHR) {
        if (!recreateSwapchain(errorMessage)) {
            Logger::error(errorMessage);
            return false;
        }
        _window->setFramebufferResized(false);
        return true;
    }
    if (queuePresent != vk::Result::eSuccess) {
        Logger::error(errorMessage);
        return false;
    }

    return true;
}

bool VulkanRenderer::recreateSwapchain(std::string& errorMessage) {
    VulkanSwapchain& swapchain = context.getSwapchain();

    if (!swapchain.recreate(context.getSurface(), errorMessage)) return false;

    syncObjects.backup();

    const vk::Device& logicalDevice       = context.getDevice().getLogicalDevice();
    const uint32_t    swapchainImageCount = swapchain.getImages().size();

    if (!syncObjects.create(logicalDevice, MAX_FRAMES_IN_FLIGHT, swapchainImageCount, errorMessage)) return false;
    return true;
}
