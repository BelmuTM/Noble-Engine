#include "VulkanRenderer.h"
#include "graphics/vulkan/common/VulkanDebugger.h"
#include "graphics/vulkan/resources/VulkanMesh.h"

#include "core/Engine.h"
#include "core/debug/Logger.h"
#include "core/debug/ErrorHandling.h"

#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static constexpr vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

VulkanRenderer::~VulkanRenderer() {
    shutdown();
}

bool VulkanRenderer::init(Platform::Window& window) {
    _window = &window;

    std::string errorMessage = "Failed to create Vulkan renderer context: no error message provided";

    ScopeGuard guard{[this, &errorMessage] {
        shutdown();
        Engine::fatalExit(errorMessage);
    }};

    // Create Vulkan context
    if (!context.create(window, errorMessage)) return false;

    // Create entities
    const VulkanDevice&    device              = context.getDevice();
    const VulkanSwapchain& swapchain           = context.getSwapchain();
    const vk::Device&      logicalDevice       = device.getLogicalDevice();
    const uint32_t         swapchainImageCount = swapchain.getImages().size();

    if (!createVulkanEntity(&commandManager, errorMessage, device, MAX_FRAMES_IN_FLIGHT)) return false;

    if (!createVulkanEntity(&syncObjects, errorMessage, logicalDevice, MAX_FRAMES_IN_FLIGHT, swapchainImageCount))
        return false;

    if (!createVulkanEntity(&uniformBuffers, errorMessage, device, MAX_FRAMES_IN_FLIGHT)) return false;

    if (!createVulkanEntity(&descriptor, errorMessage, logicalDevice, MAX_FRAMES_IN_FLIGHT)) return false;

    vk::DescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding
        .setBinding(0)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eAllGraphics);

    if (!descriptor.createSetLayout({uboLayoutBinding}, errorMessage)) return false;

    VulkanShaderProgram program(logicalDevice);
    if (!program.loadFromFiles({"meow.vert.spv", "meow.frag.spv"}, errorMessage)) {
        return false;
    }

    if (!createVulkanEntity(&pipeline, errorMessage, logicalDevice, swapchain, descriptor.getLayout(), program))
        return false;

    if (!descriptor.createPool(vk::DescriptorType::eUniformBuffer, errorMessage)) return false;
    if (!descriptor.allocateSets(errorMessage)) return false;

    uniformBuffers.bindToDescriptor(descriptor, uboLayoutBinding.binding);

    VulkanMesh mesh{};
    const std::vector meshes = {mesh};

    if (!createVulkanEntity(&meshManager, errorMessage, device, commandManager, meshes)) return false;

    DrawCall verticesDraw;
    verticesDraw.pipeline = &pipeline;
    verticesDraw.mesh     = mesh;

    FramePass mainPass;
    mainPass.name      = "MainPass";
    mainPass.bindPoint = vk::PipelineBindPoint::eGraphics;
    mainPass.drawCalls = { verticesDraw };

    frameGraph.addPass(mainPass);

    if (!createVulkanEntity(&frameGraph, errorMessage, meshManager)) return false;

    guard.release();
    return true;
}

void VulkanRenderer::shutdown() {
    VK_CALL_LOG(context.getDevice().getLogicalDevice().waitIdle(), Logger::Level::ERROR);
    flushDeletionQueue();

    context.destroy();
}

void VulkanRenderer::drawFrame() {
    bool discardLogging = false;
    std::string errorMessage;

    // Declare error logging guard
    ScopeGuard guard{[&discardLogging, &errorMessage] {
        if (!discardLogging) Logger::error(errorMessage);
    }};

    // Record frame
    if (!handleFramebufferResize(errorMessage)) return;

    const auto imageIndexOpt = acquireNextImage(errorMessage, discardLogging);
    if (!imageIndexOpt) return;

    const uint32_t imageIndex = *imageIndexOpt;

    waitForImageFence(imageIndex);
    recordCurrentCommandBuffer(imageIndex);

    updateUniformBuffer();

    if (!submitCommandBuffer(imageIndex, errorMessage, discardLogging)) return;

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    guard.release();
}

bool VulkanRenderer::handleFramebufferResize(std::string& errorMessage) {
    if (!_window->isFramebufferResized()) return true;

    if (!recreateSwapchain(errorMessage)) return false;

    _window->setFramebufferResized(false);
    return true;
}

std::optional<uint32_t> VulkanRenderer::acquireNextImage(std::string& errorMessage, bool& discardLogging) {
    const vk::Device&      logicalDevice = context.getDevice().getLogicalDevice();
    const VulkanSwapchain& swapchain     = context.getSwapchain();

    if (swapchain.handle() == VK_NULL_HANDLE) {
        discardLogging = true;
        return std::nullopt;
    }

    const vk::Semaphore& imageAvailableSemaphore = syncObjects.imageAvailableSemaphores[currentFrame];
    const vk::Fence&     inFlightFence           = syncObjects.inFlightFences[currentFrame];

    while (vk::Result::eTimeout == logicalDevice.waitForFences(inFlightFence, vk::True, UINT64_MAX)) {}

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

    recordCommandBuffer(currentBuffer, imageIndex);
}

bool VulkanRenderer::submitCommandBuffer(const uint32_t imageIndex, std::string& errorMessage, bool& discardLogging) {
    const VulkanDevice&     device    = context.getDevice();
    const vk::SwapchainKHR& swapchain = context.getSwapchain().handle();

    if (swapchain == VK_NULL_HANDLE) {
        discardLogging = true;
        return false;
    }

    const vk::CommandBuffer& currentBuffer = commandManager.getCommandBuffers()[currentFrame];

    const vk::Semaphore& imageAvailableSemaphore = syncObjects.imageAvailableSemaphores[currentFrame];
    const vk::Fence&     inFlightFence           = syncObjects.inFlightFences[currentFrame];
    const vk::Semaphore& renderFinishedSemaphore = syncObjects.renderFinishedSemaphores[imageIndex];

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
        if (!recreateSwapchain(errorMessage)) return false;
        _window->setFramebufferResized(false);
        return true;
    }
    if (queuePresent != vk::Result::eSuccess) return false;

    return true;
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
) const {
    if (oldLayout == newLayout) return;

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
        .setImageMemoryBarriers(barrier);

    commandBuffer.pipelineBarrier2(dependencyInfo);
}

bool VulkanRenderer::beginCommandBuffer(const vk::CommandBuffer commandBuffer, std::string& errorMessage) {
    if (commandBuffer == vk::CommandBuffer{}) {
        errorMessage = "Failed to beging record for Vulkan command buffer: command buffer is null";
        return false;
    }

    constexpr vk::CommandBufferBeginInfo beginInfo{};
    VK_TRY_LOG(commandBuffer.begin(beginInfo), Logger::Level::ERROR);
    return true;
}

void VulkanRenderer::recordCommandBuffer(const vk::CommandBuffer commandBuffer, const uint32_t imageIndex) {
    std::string errorMessage;

    if (!beginCommandBuffer(commandBuffer, errorMessage)) {
        Logger::error(errorMessage);
        return;
    }

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

    const VulkanSwapchain& swapchain = context.getSwapchain();
    const vk::Extent2D&    extent    = swapchain.getExtent2D();

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
        .setColorAttachments(attachmentInfo);

    const vk::Viewport& viewport = {
        0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f
    };

    const auto scissor = vk::Rect2D(vk::Offset2D(0, 0), extent);

    for (auto& pass : frameGraph.getPasses()) {
        pass.renderingInfo = renderingInfo;

        for (auto& draw : pass.drawCalls) {
            draw.descriptorSets = { descriptor.getDescriptorSets()[currentFrame] };
            draw.viewport       = viewport;
            draw.scissor        = scissor;
        }

        frameGraph.executePass(pass, commandBuffer);
    }

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

bool VulkanRenderer::recreateSwapchain(std::string& errorMessage) {
    VulkanSwapchain& swapchain = context.getSwapchain();
    if (!swapchain.recreate(context.getSurface(), errorMessage)) return false;

    syncObjects.backup();

    const vk::Device& logicalDevice       = context.getDevice().getLogicalDevice();
    const uint32_t    swapchainImageCount = swapchain.getImages().size();

    if (!syncObjects.create(logicalDevice, MAX_FRAMES_IN_FLIGHT, swapchainImageCount, errorMessage)) return false;
    return true;
}

void VulkanRenderer::updateUniformBuffer() {
    static auto startTime = std::chrono::high_resolution_clock::now();

    const auto  currentTime      = std::chrono::high_resolution_clock::now();
    const float frameTimeCounter = std::chrono::duration<float>(currentTime - startTime).count();

    const vk::Extent2D& extent      = context.getSwapchain().getExtent2D();
    const float         aspectRatio = static_cast<float>(extent.width) / static_cast<float>(extent.height);

    UniformBufferObject ubo{};
    ubo.model      = glm::rotate(glm::mat4(1.0f), frameTimeCounter * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view       = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 10.0f);

    ubo.projection[1][1] *= -1;

    memcpy(uniformBuffers.getBuffers()[currentFrame].getMappedPointer(), &ubo, sizeof(ubo));
}
