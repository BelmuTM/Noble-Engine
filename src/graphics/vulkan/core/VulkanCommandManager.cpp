#include "VulkanCommandManager.h"

#include "core/debug/Logger.h"
#include "graphics/vulkan/common/VulkanDebugger.h"
#include "graphics/vulkan/pipeline/VulkanGraphicsPipeline.h"

static constexpr vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

bool VulkanCommandManager::create(
    const VulkanDevice& device, const VulkanSwapchain& swapchain, const uint32_t commandBufferCount,
    std::string& errorMessage
) noexcept {
    _device    = &device;
    _swapchain = &swapchain;

    if (!createCommandPool(errorMessage)) return false;
    if (!createCommandBuffers(commandBufferCount, errorMessage)) return false;
    return true;
}

void VulkanCommandManager::destroy() noexcept {
    if (!_device) return;

    if (commandPool) {
        _device->getLogicalDevice().destroyCommandPool(commandPool);
        commandPool = VK_NULL_HANDLE;
    }

    _device    = nullptr;
    _swapchain = nullptr;
}

bool VulkanCommandManager::createCommandPool(std::string& errorMessage) {
    vk::CommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(_device->getQueueFamilyIndices().graphicsFamily);

    const vk::Device& logicalDevice = _device->getLogicalDevice();

    VK_CREATE(logicalDevice.createCommandPool(commandPoolInfo), commandPool, errorMessage);
    return true;
}

bool VulkanCommandManager::createCommandBuffers(const uint32_t commandBufferCount, std::string& errorMessage) {
    vk::CommandBufferAllocateInfo allocateInfo{};
    allocateInfo
        .setCommandPool(commandPool)
        .setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandBufferCount(commandBufferCount);

    const vk::Device& logicalDevice = _device->getLogicalDevice();

    VK_CREATE(logicalDevice.allocateCommandBuffers(allocateInfo), commandBuffers, errorMessage);
    return true;
}

void VulkanCommandManager::transitionImageLayout(
    const vk::CommandBuffer       commandBuffer,
    const uint32_t                imageIndex,
    const vk::ImageLayout         oldLayout,
    const vk::ImageLayout         newLayout,
    const vk::AccessFlags2        srcAccessMask,
    const vk::AccessFlags2        dstAccessMask,
    const vk::PipelineStageFlags2 srcStageMask,
    const vk::PipelineStageFlags2 dstStageMask
) const {
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
        .setImage(_swapchain->getImages()[imageIndex])
        .setSubresourceRange(subresourceRange);

    vk::DependencyInfo dependencyInfo{};
    dependencyInfo
        .setDependencyFlags({})
        .setImageMemoryBarrierCount(1) // Can have multiple barriers per transition
        .setPImageMemoryBarriers(&barrier);

    commandBuffer.pipelineBarrier2(dependencyInfo);
}

void VulkanCommandManager::recordCommandBuffer(
    const vk::CommandBuffer commandBuffer, const uint32_t imageIndex, const VulkanGraphicsPipeline& pipeline
) const {
    if (commandBuffer == vk::CommandBuffer{}) {
        Logger::error("Failed to record Vulkan command buffer: command buffer is null");
        return;
    }

    constexpr vk::CommandBufferBeginInfo beginInfo{};
    VK_TRY_VOID_LOG(commandBuffer.begin(beginInfo), Logger::Level::ERROR);

    const vk::Extent2D& extent = _swapchain->getExtent2D();

    const vk::Viewport& viewport = {
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
        .setImageView(_swapchain->getImageViews()[imageIndex])
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

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

    const vk::Buffer&     vertexBuffer = pipeline.getVertexBuffer();
    const vk::DeviceSize& offset       = 0;
    commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer, &offset);

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
