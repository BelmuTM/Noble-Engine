#include "VulkanRenderer.h"

#include "graphics/vulkan/common/VulkanDebugger.h"
#include "graphics/vulkan/resources/images/VulkanImageLayoutTransitions.h"

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

    TRY(createVulkanEntity(&frameGraph, errorMessage, meshManager, device.getQueryPool()));

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

    if (!recordCurrentCommandBuffer(imageIndex, errorMessage)) return;

    frameResources.update(currentFrame, camera);
    renderObjectManager.updateObjects();

    if (!submitCurrentCommandBuffer(currentFrame, imageIndex, errorMessage, discardLogging)) return;

    currentFrame = (currentFrame + 1) % _framesInFlight;

    uint64_t primitiveCount = 0;

    vk::Result queryResults = context.getDevice().getLogicalDevice().getQueryPoolResults(
        context.getDevice().getQueryPool(),
        0, 1,
        sizeof(uint64_t),
        &primitiveCount,
        sizeof(uint64_t),
        vk::QueryResultFlagBits::eWait |
        vk::QueryResultFlagBits::e64
    );

    //Logger::debug("Drew " + std::to_string(primitiveCount) + " triangles");

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

bool VulkanRenderer::recordCommandBuffer(
    const vk::CommandBuffer commandBuffer, const uint32_t imageIndex, std::string& errorMessage
) {
    if (commandBuffer == vk::CommandBuffer{}) {
        errorMessage = "Failed to record Vulkan command buffer: command buffer is null";
        return false;
    }

    constexpr vk::CommandBufferBeginInfo beginInfo{};
    VK_TRY(commandBuffer.begin(beginInfo), errorMessage);

    const VulkanSwapchain& swapchain = context.getSwapchain();

    const vk::Extent2D swapchainExtent = swapchain.getExtent();
    const vk::Format   swapchainFormat = swapchain.getFormat();

    const vk::Image&     swapchainImage     = swapchain.getImages()[imageIndex];
    const vk::ImageView& swapchainImageView = swapchain.getImageViews()[imageIndex];

    TRY(VulkanImageLayoutTransitions::transitionImageLayout(
        swapchainImage,
        swapchainFormat,
        commandBuffer,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        1,
        errorMessage
    ));

    FrameContext frameContext{};
    frameContext
        .setFrameIndex(currentFrame)
        .setCommandBuffer(commandBuffer)
        .setSwapchainImageView(swapchainImageView)
        .setExtent(swapchainExtent);

    frameContext.frameDescriptors = frameResources.getUBODescriptors()->getSets();

    frameGraph.execute(frameContext);

    TRY(VulkanImageLayoutTransitions::transitionImageLayout(
        swapchainImage,
        swapchainFormat,
        commandBuffer,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        1,
        errorMessage
    ));

    VK_TRY(commandBuffer.end(), errorMessage);

    return true;
}

bool VulkanRenderer::recordCurrentCommandBuffer(const uint32_t imageIndex, std::string& errorMessage) {
    const vk::CommandBuffer& currentBuffer = commandManager.getCommandBuffers()[currentFrame];
    VK_CALL_LOG(currentBuffer.reset(), Logger::Level::ERROR);

    TRY(recordCommandBuffer(currentBuffer, imageIndex, errorMessage));

    return true;
}

bool VulkanRenderer::submitCurrentCommandBuffer(
    const uint32_t frameIndex, const uint32_t imageIndex, std::string& errorMessage, bool& discardLogging
) {
    const vk::CommandBuffer& currentBuffer = commandManager.getCommandBuffers()[currentFrame];

    TRY(swapchainManager.submitCommandBuffer(currentBuffer, frameIndex, imageIndex, errorMessage, discardLogging));

    return true;
}
