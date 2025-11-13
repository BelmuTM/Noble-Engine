#include "VulkanRenderer.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "core/debug/ErrorHandling.h"
#include "core/debug/Logger.h"

VulkanRenderer::VulkanRenderer(const uint32_t framesInFlight) : _framesInFlight(framesInFlight) {}

VulkanRenderer::~VulkanRenderer() {
    shutdown();
}

bool VulkanRenderer::init(Platform::Window& window, const ObjectsVector& objects, std::string& errorMessage) {
    _window = &window;

    errorMessage = "Failed to init Vulkan renderer: no error message provided";

    ScopeGuard guard{[this] { shutdown(); }};

    // Create Vulkan context
    TRY(context.create(window, errorMessage));

    const VulkanSurface& surface       = context.getSurface();
    const VulkanDevice&  device        = context.getDevice();
    const vk::Device&    logicalDevice = device.getLogicalDevice();

    VulkanSwapchain& swapchain = context.getSwapchain();

    // Create resource managers
    TRY(createVulkanEntity(&swapchainManager, errorMessage, window, surface, device, swapchain, _framesInFlight));
    TRY(createVulkanEntity(&commandManager, errorMessage, device, _framesInFlight));
    TRY(createVulkanEntity(&pipelineManager, errorMessage, logicalDevice, swapchain));
    TRY(createVulkanEntity(&imageManager, errorMessage, device, commandManager));
    TRY(createVulkanEntity(&uniformBufferManager, errorMessage, device, _framesInFlight));

    TRY(createVulkanEntity(
        &frameResources, errorMessage, device, swapchain, imageManager, uniformBufferManager, _framesInFlight
    ));

    TRY(createVulkanEntity(
        &renderObjectManager, errorMessage, objects, device, imageManager, meshManager, _framesInFlight
    ));

    TRY(createVulkanEntity(&meshManager, errorMessage, device, commandManager, renderObjectManager.getMeshes()));

    // Create rendering pipeline
    const std::vector descriptorLayoutsMeshRender = {
        frameResources.getDescriptorManager().getLayout(), renderObjectManager.getDescriptorManager().getLayout()
    };

    VulkanShaderProgram meshRender(logicalDevice);
    TRY(meshRender.load("mesh_render", false, errorMessage));

    TRY(pipelineManager.createGraphicsPipeline(
        pipelineMeshRender, descriptorLayoutsMeshRender, objectDataGPUSize, meshRender, errorMessage
    ));

    FrameResource swapchainOutput{};
    swapchainOutput
        .setType(SwapchainOutput)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setResolveImageView([](const FrameContext& frame) { return frame.swapchainImageView; });

    FramePassAttachment swapchainAttachment{};
    swapchainAttachment
        .setResource(swapchainOutput)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(defaultClearColor);

    FrameResource depthBuffer{};
    depthBuffer
        .setType(Buffer)
        .setImage(frameResources.getDepthBuffer())
        .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
        .setResolveImageView([this](const FrameContext&) { return frameResources.getDepthBuffer().getImageView(); });

    FramePassAttachment depthAttachment{};
    depthAttachment
        .setResource(depthBuffer)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(vk::ClearDepthStencilValue{1.0f, 0});

    /*
    std::vector descriptorLayoutsComposite  = {frameResources.getDescriptorManager().getLayout()};

    VulkanShaderProgram composite(logicalDevice);
    TRY(composite.load("composite", true, errorMessage));

    TRY(pipelineManager.createGraphicsPipeline(pipelineComposite, descriptorLayoutsComposite, composite, errorMessage));

    TRY(imageManager.createColorBuffer(
        compositeOutput,
        swapchain.getExtent(),
        vk::Format::eR8G8B8A8Unorm,
        errorMessage
    ));

    FrameResource compositeBuffer{};
    compositeBuffer
        .setType(Buffer)
        .setImage(compositeOutput)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setResolveImageView([this](const FrameContext&) { return compositeOutput.getImageView(); });

    FramePassAttachment compositeAttachment{};
    compositeAttachment
        .setResource(compositeBuffer)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f});

    // TO-DO: Color attachment helper class
    FramePass compositePass;
    compositePass
        .setName("Composite_Pass")
        .setPipeline(&pipelineComposite)
        .setBindPoint(vk::PipelineBindPoint::eGraphics)
        //.addColorAttachment(compositeAttachment)
        .setDepthAttachment(depthAttachment);

    DrawCall fullscreenDraw;
    fullscreenDraw.setMesh(VulkanMesh::makeFullscreenTriangle());

    compositePass.addDrawCall(fullscreenDraw);
    */

    FramePass meshRenderPass;
    meshRenderPass
        .setName("MeshRender_Pass")
        .setPipeline(&pipelineMeshRender)
        .setBindPoint(vk::PipelineBindPoint::eGraphics)
        .addColorAttachment(swapchainAttachment)
        .setDepthAttachment(depthAttachment);

    for (const auto& renderObject : renderObjectManager.getRenderObjects()) {
        meshRenderPass.addObjectDrawCall(renderObject.get());
    }

    frameGraph.addPass(std::move(meshRenderPass));

    TRY(createVulkanEntity(&frameGraph, errorMessage, meshManager));

    guard.release();

    return true;
}

void VulkanRenderer::shutdown() {
    VK_CALL_LOG(context.getDevice().getLogicalDevice().waitIdle(), Logger::Level::ERROR);
    flushDeletionQueue();

    context.destroy();
}

void VulkanRenderer::drawFrame(const Camera& camera) {
    bool discardLogging = swapchainManager.isOutOfDate();
    std::string errorMessage;

    ScopeGuard guard{[&discardLogging, &errorMessage] {
        if (!discardLogging) Logger::error(errorMessage);
    }};

    if (!onFramebufferResize(errorMessage)) return;

    const auto imageIndexOpt = swapchainManager.acquireNextImage(currentFrame, errorMessage, discardLogging);
    if (!imageIndexOpt) return;

    const uint32_t imageIndex = *imageIndexOpt;

    recordCurrentCommandBuffer(imageIndex);

    frameResources.update(currentFrame, camera);
    renderObjectManager.updateObjects();

    if (!swapchainManager.submitCommandBuffer(commandManager, currentFrame, imageIndex, errorMessage, discardLogging)) {
        return;
    }

    currentFrame = (currentFrame + 1) % _framesInFlight;
    guard.release();
}

bool VulkanRenderer::onFramebufferResize(std::string& errorMessage) {
    if (!_window) return false;
    if (!_window->isFramebufferResized()) return true;

    VK_CALL_LOG(context.getDevice().getLogicalDevice().waitIdle(), Logger::Level::ERROR);

    TRY(swapchainManager.recreateSwapchain(errorMessage));
    TRY(frameResources.recreate(errorMessage));

    _window->setFramebufferResized(false);

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
        errorMessage = "Failed to begin record for Vulkan command buffer: command buffer is null";
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

    FrameContext frameContext{};
    frameContext
        .setFrameIndex(currentFrame)
        .setCommandBuffer(commandBuffer)
        .setSwapchainImageView(context.getSwapchain().getImageViews()[imageIndex])
        .setExtent(context.getSwapchain().getExtent());

    frameContext.frameDescriptors = frameResources.getUBODescriptors()->getSets();

    frameGraph.execute(frameContext);

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

void VulkanRenderer::recordCurrentCommandBuffer(const uint32_t imageIndex) {
    const vk::CommandBuffer& currentBuffer = commandManager.getCommandBuffers()[currentFrame];
    VK_CALL_LOG(currentBuffer.reset(), Logger::Level::ERROR);

    recordCommandBuffer(currentBuffer, imageIndex);
}
