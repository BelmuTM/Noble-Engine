#include "VulkanRenderer.h"

#include <iostream>

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "core/Engine.h"
#include "core/debug/Logger.h"

static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

static constexpr vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

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

    if (!context.create(window, errorMessage)) return false;
    if (!createCommandPool(errorMessage)) return false;
    if (!createCommandBuffer(errorMessage)) return false;
    if (!createSyncObjects(errorMessage)) return false;

    if (!graphicsPipeline.create(&context.getDevice().getLogicalDevice(), context.getSwapchain(), errorMessage)) {
        return false;
    }

    imagesInFlight.resize(context.getSwapchain().getImages().size(), VK_NULL_HANDLE);

    guard.release();
    return true;
}

void VulkanRenderer::shutdown() {
    const vk::Device& logicalDevice = context.getDevice().getLogicalDevice();

    cleanupOldSyncObjects();
    destroySyncObjects();

    if (commandPool) {
        logicalDevice.destroyCommandPool(commandPool);
        commandPool = nullptr;
    }

    graphicsPipeline.destroy();
    context.destroy();
}

void VulkanRenderer::drawFrame() {
    std::string errorMessage;

    VulkanDevice      device        = context.getDevice();
    const vk::Device& logicalDevice = device.getLogicalDevice();
    vk::SwapchainKHR  swapchain     = context.getSwapchain();

    while (vk::Result::eTimeout == logicalDevice.waitForFences(inFlightFences[currentFrame], vk::True, UINT64_MAX)) {}

    const auto nextImageAcquire =
        VK_CALL(logicalDevice.acquireNextImageKHR(swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame],
                nullptr), errorMessage);

    if (nextImageAcquire.result == vk::Result::eErrorOutOfDateKHR ||
        nextImageAcquire.result == vk::Result::eSuboptimalKHR) {

        if (!recreateSwapchain(errorMessage)) {
            Logger::error(errorMessage);
            return;
        }

        _window->setFramebufferResized(false);
        return;
    }

    if (nextImageAcquire.result != vk::Result::eSuccess) {
        Logger::error(VulkanDebugger::formatVulkanErrorMessage("acquireNextImageKHR", nextImageAcquire.result));
        return;
    }

    uint32_t imageIndex = nextImageAcquire.value;

    if (imageIndex >= context.getSwapchain().getImages().size()) {
        Logger::error("Failed to record Vulkan command buffer: image index exceeds limit");
        return;
    }

    if (imagesInFlight[imageIndex]) {
        VK_CALL_LOG(logicalDevice.waitForFences(imagesInFlight[imageIndex], vk::True, UINT64_MAX),
                    Logger::Level::ERROR);
    }

    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    VK_TRY_VOID_LOG(logicalDevice.resetFences(inFlightFences[currentFrame]), Logger::Level::ERROR);

    VK_CALL_LOG(commandBuffers[currentFrame].reset(), Logger::Level::ERROR);

    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    constexpr vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    vk::SubmitInfo submitInfo{};
    submitInfo
        .setWaitSemaphoreCount(1)
        .setPWaitSemaphores(&imageAvailableSemaphores[currentFrame])
        .setPWaitDstStageMask(&waitDestinationStageMask)
        .setCommandBufferCount(1)
        .setPCommandBuffers(&commandBuffers[currentFrame])
        .setSignalSemaphoreCount(1)
        .setPSignalSemaphores(&renderFinishedSemaphores[imageIndex]);

    VK_TRY_VOID_LOG(device.getGraphicsQueue().submit(submitInfo, inFlightFences[currentFrame]), Logger::Level::ERROR);

    const vk::PresentInfoKHR presentInfo(renderFinishedSemaphores[imageIndex], swapchain, imageIndex);
    const auto queuePresent = VK_CALL_LOG(device.getPresentQueue().presentKHR(presentInfo), Logger::Level::ERROR);

    if (queuePresent == vk::Result::eErrorOutOfDateKHR || queuePresent == vk::Result::eSuboptimalKHR) {
        if (!recreateSwapchain(errorMessage)) {
            Logger::error(errorMessage);
            return;
        }
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

bool VulkanRenderer::recreateSwapchain(std::string& errorMessage) {
    if (!context.getSwapchain().recreate(context.getSurface(), errorMessage)) return false;

    oldFences.insert(oldFences.end(), inFlightFences.begin(), inFlightFences.end());
    oldImageAvailable.insert(oldImageAvailable.end(), imageAvailableSemaphores.begin(), imageAvailableSemaphores.end());
    oldRenderFinished.insert(oldRenderFinished.end(), renderFinishedSemaphores.begin(), renderFinishedSemaphores.end());

    if (!createSyncObjects(errorMessage)) return false;
    return true;
}


bool VulkanRenderer::createCommandPool(std::string& errorMessage) {
    VulkanDevice device = context.getDevice();

    vk::CommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(device.getQueueFamilyIndices().graphicsFamily);

    const auto commandPoolCreate =
        VK_CALL(device.getLogicalDevice().createCommandPool(commandPoolInfo), errorMessage);

    if (commandPoolCreate.result != vk::Result::eSuccess) return false;

    commandPool = commandPoolCreate.value;
    return true;
}

bool VulkanRenderer::createCommandBuffer(std::string& errorMessage) {
    vk::CommandBufferAllocateInfo allocateInfo{};
    allocateInfo
        .setCommandPool(commandPool)
        .setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandBufferCount(MAX_FRAMES_IN_FLIGHT);

    const vk::Device& logicalDevice = context.getDevice().getLogicalDevice();

    const auto commandBuffersAllocate =
        VK_CALL(logicalDevice.allocateCommandBuffers(allocateInfo), errorMessage);
    if (commandBuffersAllocate.result != vk::Result::eSuccess) return false;

    commandBuffers = commandBuffersAllocate.value;
    return true;
}

bool VulkanRenderer::createSyncObjects(std::string& errorMessage) {
    const vk::Device& logicalDevice = context.getDevice().getLogicalDevice();

    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(context.getSwapchain().getImages().size());
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    constexpr vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VK_CREATE(logicalDevice.createSemaphore({}), imageAvailableSemaphores[i], errorMessage);
        VK_CREATE(logicalDevice.createFence(fenceInfo), inFlightFences[i], errorMessage);
    }

    for (size_t i = 0; i < renderFinishedSemaphores.size(); i++) {
        VK_CREATE(logicalDevice.createSemaphore({}), renderFinishedSemaphores[i], errorMessage);
    }
    return true;
}

void VulkanRenderer::destroySyncObjects() {
    const vk::Device& logicalDevice = context.getDevice().getLogicalDevice();

    for (const auto& imageSemaphore : imageAvailableSemaphores) {
        logicalDevice.destroySemaphore(imageSemaphore);
    }

    for (const auto& renderSemaphore : renderFinishedSemaphores) {
        logicalDevice.destroySemaphore(renderSemaphore);
    }

    for (const auto& fence : inFlightFences) {
        logicalDevice.destroyFence(fence);
    }

    imageAvailableSemaphores.clear();
    renderFinishedSemaphores.clear();
    inFlightFences.clear();
}

void VulkanRenderer::cleanupOldSyncObjects() {
    const vk::Device& logicalDevice = context.getDevice().getLogicalDevice();

    for (auto it = oldFences.begin(); it != oldFences.end(); ) {
        if (logicalDevice.getFenceStatus(*it) == vk::Result::eSuccess) {
            logicalDevice.destroyFence(*it);
            it = oldFences.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = oldImageAvailable.begin(); it != oldImageAvailable.end(); ) {
        logicalDevice.destroySemaphore(*it);
        it = oldImageAvailable.erase(it);
    }

    for (auto it = oldRenderFinished.begin(); it != oldRenderFinished.end(); ) {
        logicalDevice.destroySemaphore(*it);
        it = oldRenderFinished.erase(it);
    }
}

void VulkanRenderer::transitionImageLayout(
    const vk::CommandBuffer       commandBuffer,
    const uint32_t                imageIndex,
    const vk::ImageLayout         oldLayout,
    const vk::ImageLayout         newLayout,
    const vk::AccessFlags2        srcAccessMask,
    const vk::AccessFlags2        dstAccessMask,
    const vk::PipelineStageFlags2 srcStageMask,
    const vk::PipelineStageFlags2 dstStageMask
) {
    // Specify which part of the image to transition
    vk::ImageSubresourceRange subresourceRange{};
    subresourceRange
        .setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setBaseMipLevel(0)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    // Define how the transition operates and what to change
    vk::ImageMemoryBarrier2 barrier{};
    barrier
        .setSrcStageMask(srcStageMask)
        .setSrcAccessMask(srcAccessMask)
        .setDstStageMask(dstStageMask)
        .setDstAccessMask(dstAccessMask)
        .setOldLayout(oldLayout)
        .setNewLayout(newLayout)
        .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored) // No ownership transfer to another queue family
        .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
        .setImage(context.getSwapchain().getImages()[imageIndex])
        .setSubresourceRange(subresourceRange);

    vk::DependencyInfo dependencyInfo{};
    dependencyInfo
        .setDependencyFlags({})
        .setImageMemoryBarrierCount(1) // Can have multiple barriers per transition
        .setPImageMemoryBarriers(&barrier);

    commandBuffer.pipelineBarrier2(dependencyInfo);
}

void VulkanRenderer::recordCommandBuffer(const vk::CommandBuffer commandBuffer, const uint32_t imageIndex) {
    if (commandBuffer == vk::CommandBuffer{}) {
        Logger::error("Failed to record Vulkan command buffer: command buffer is null");
        return;
    }

    constexpr vk::CommandBufferBeginInfo beginInfo{};
    VK_TRY_VOID_LOG(commandBuffer.begin(beginInfo), Logger::Level::ERROR);

    const VulkanSwapchain& swapchain = context.getSwapchain();
    const vk::Extent2D     extent    = swapchain.getExtent2D();

    const vk::Viewport viewport = {
        0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f
    };

    const auto scissor = vk::Rect2D(vk::Offset2D(0, 0), extent);

    transitionImageLayout(
        commandBuffer,
        imageIndex,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eTopOfPipe,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput
    );

    vk::RenderingAttachmentInfo attachmentInfo{};
    attachmentInfo
        .setImageView(swapchain.getImageViews()[imageIndex])
        .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(clearColor);

    vk::RenderingInfo renderingInfo{};
    renderingInfo
        .setRenderArea({{0, 0}, extent})
        .setLayerCount(1)
        .setColorAttachmentCount(1)
        .setPColorAttachments(&attachmentInfo);

    commandBuffer.beginRendering(renderingInfo);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    commandBuffer.setViewport(0, viewport);
    commandBuffer.setScissor(0, scissor);

    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRendering();

    transitionImageLayout(
        commandBuffer,
        imageIndex,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe
    );

    VK_CALL_LOG(commandBuffer.end(), Logger::Level::ERROR);
}
