#include "VulkanRenderer.h"

#include <iostream>

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "core/Engine.h"
#include "core/debug/Logger.h"

VulkanRenderer::~VulkanRenderer() {
    shutdown();
}

bool VulkanRenderer::init(const Platform::Window& window) {
    std::string errorMessage = "Failed to create Vulkan context: no error message provided";
    if (!context.create(window, errorMessage)) {
        Engine::fatalExit(errorMessage);
    }

    if (!createSyncObjects(errorMessage)) {
        Engine::fatalExit(errorMessage);
    }

    if (!createCommandPool(errorMessage)) {
        Engine::fatalExit(errorMessage);
    }

    if (!createCommandBuffer(errorMessage)) {
        Engine::fatalExit(errorMessage);
    }

    if (!graphicsPipeline.create(&context.getDevice().getLogicalDevice(), context.getSwapchain(), errorMessage)) {
        Engine::fatalExit(errorMessage);
    }
    return true;
}

void VulkanRenderer::shutdown() {
    const vk::Device& logicalDevice = context.getDevice().getLogicalDevice();

    if (imageAvailableSemaphore) {
        logicalDevice.destroySemaphore(imageAvailableSemaphore);
        imageAvailableSemaphore = nullptr;
    }

    for (const auto& semaphore : renderFinishedSemaphores) {
        logicalDevice.destroySemaphore(semaphore);
    }

    if (drawFence) {
        logicalDevice.destroyFence(drawFence);
        drawFence = nullptr;
    }

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

    const auto nextImageAcquire = VK_CHECK_RESULT(
        logicalDevice.acquireNextImageKHR(swapchain, UINT64_MAX, imageAvailableSemaphore, nullptr),
        errorMessage
        );

    if (nextImageAcquire.result != vk::Result::eSuccess) {
        Logger::error("Failed to acquire Vulkan swapchain image");
        return;
    }

    const uint32_t imageIndex = nextImageAcquire.value;
    if (imageIndex >= context.getSwapchain().getImages().size()) {
        Logger::error("Failed to record Vulkan command buffer: image index exceeds limit");
        return;
    }

    recordCommandBuffer(imageIndex);

    const vk::Result fencesReset = VK_CHECK_RESULT(logicalDevice.resetFences(drawFence), errorMessage);
    if (fencesReset != vk::Result::eSuccess) {
        Logger::error(errorMessage);
        return;
    }

    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    const vk::SubmitInfo submitInfo(imageAvailableSemaphore, waitDestinationStageMask, commandBuffer,
                                    renderFinishedSemaphores[imageIndex]);

    const auto commandBufferSubmit =
        VK_CHECK_RESULT(device.getGraphicsQueue().submit(submitInfo, drawFence), errorMessage);

    if (commandBufferSubmit != vk::Result::eSuccess) {
        Logger::error(errorMessage);
        return;
    }

    while (vk::Result::eTimeout == logicalDevice.waitForFences(drawFence, vk::True, UINT64_MAX)) {}

    const vk::PresentInfoKHR presentInfo(renderFinishedSemaphores[imageIndex], swapchain, imageIndex);

    const vk::Result imagePresent = device.getPresentQueue().presentKHR(presentInfo);
    if (imagePresent != vk::Result::eSuccess) {
        Logger::error(errorMessage);
    }
}

bool VulkanRenderer::createCommandPool(std::string& errorMessage) {
    VulkanDevice device = context.getDevice();

    vk::CommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(device.getQueueFamilyIndices().graphicsFamily);

    const auto commandPoolCreate =
        VK_CHECK_RESULT(device.getLogicalDevice().createCommandPool(commandPoolInfo), errorMessage);

    if (commandPoolCreate.result != vk::Result::eSuccess) return false;

    commandPool = commandPoolCreate.value;
    return true;
}

bool VulkanRenderer::createCommandBuffer(std::string& errorMessage) {
    vk::CommandBufferAllocateInfo allocateInfo{};
    allocateInfo
        .setCommandPool(commandPool)
        .setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandBufferCount(1);

    const vk::Device& logicalDevice = context.getDevice().getLogicalDevice();

    const auto commandBuffersAllocate =
        VK_CHECK_RESULT(logicalDevice.allocateCommandBuffers(allocateInfo), errorMessage);
    if (commandBuffersAllocate.result != vk::Result::eSuccess) return false;

    commandBuffer = commandBuffersAllocate.value.front();
    return true;
}

bool VulkanRenderer::createSyncObjects(std::string& errorMessage) {
    const auto imageAvailableSemaphoreCreate =
        VK_CHECK_RESULT(context.getDevice().getLogicalDevice().createSemaphore({}), errorMessage);
    if (imageAvailableSemaphoreCreate.result != vk::Result::eSuccess) return false;

    imageAvailableSemaphore = imageAvailableSemaphoreCreate.value;

    renderFinishedSemaphores.resize(context.getSwapchain().getImages().size());

    for (size_t i = 0; i < renderFinishedSemaphores.size(); i++) {
        const auto renderFinishedSemaphoreCreate =
            VK_CHECK_RESULT(context.getDevice().getLogicalDevice().createSemaphore({}), errorMessage);
        if (renderFinishedSemaphoreCreate.result != vk::Result::eSuccess) return false;

        renderFinishedSemaphores[i] = renderFinishedSemaphoreCreate.value;
    }

    constexpr vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};

    const auto drawFenceCreate =
        VK_CHECK_RESULT(context.getDevice().getLogicalDevice().createFence(fenceInfo), errorMessage);
    if (drawFenceCreate.result != vk::Result::eSuccess) return false;

    drawFence = drawFenceCreate.value;
    return true;
}

void VulkanRenderer::transitionImageLayout(
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

void VulkanRenderer::recordCommandBuffer(const uint32_t imageIndex) {
    std::string errorMessage;

    if (commandBuffer == vk::CommandBuffer{}) {
        Logger::error("Failed to record Vulkan command buffer: command buffer is null");
        return;
    }

    constexpr vk::CommandBufferBeginInfo beginInfo{};

    const vk::Result commandBufferBegin = VK_CHECK_RESULT(commandBuffer.begin(beginInfo), errorMessage);
    if (commandBufferBegin != vk::Result::eSuccess) {
        Logger::error(errorMessage);
        return;
    }

    transitionImageLayout(
        imageIndex,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eTopOfPipe,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput
    );

    constexpr vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    vk::RenderingAttachmentInfo attachmentInfo{};
    attachmentInfo
        .setImageView(context.getSwapchain().getImageViews()[imageIndex])
        .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(clearColor);

    vk::RenderingInfo renderingInfo{};
    renderingInfo
        .setRenderArea({{0, 0}, context.getSwapchain().getExtent2D()})
        .setLayerCount(1)
        .setColorAttachmentCount(1)
        .setPColorAttachments(&attachmentInfo);

    commandBuffer.beginRendering(renderingInfo);

    const vk::Extent2D extent = context.getSwapchain().getExtent2D();

    const vk::Viewport viewport = {
        0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f
    };

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    commandBuffer.setViewport(0, viewport);
    commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), extent));

    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRendering();

    transitionImageLayout(
        imageIndex,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe
    );

    const vk::Result commandBufferEnd = VK_CHECK_RESULT(commandBuffer.end(), errorMessage);
    if (commandBufferEnd != vk::Result::eSuccess) {
        Logger::error(errorMessage);
    }
}
