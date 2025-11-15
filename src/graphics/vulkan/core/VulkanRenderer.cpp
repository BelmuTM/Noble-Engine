#include "VulkanRenderer.h"

#include "graphics/vulkan/common/VulkanDebugger.h"
#include "graphics/vulkan/resources/images/VulkanImageLayoutTransitions.h"

#include "core/debug/ErrorHandling.h"
#include "core/debug/Logger.h"
#include "graphics/vulkan/pipeline/framegraph/passes/MeshRenderPass.h"

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

    // Resource managers creation
    TRY(createVulkanEntity(&swapchainManager, errorMessage, window, surface, device, swapchain, _framesInFlight));
    TRY(createVulkanEntity(&commandManager, errorMessage, device, _framesInFlight));
    TRY(createVulkanEntity(&imageManager, errorMessage, device, commandManager));
    TRY(createVulkanEntity(&uniformBufferManager, errorMessage, device, _framesInFlight));

    TRY(createVulkanEntity(
        &frameResources, errorMessage, device, swapchain, imageManager, uniformBufferManager, _framesInFlight
    ));

    TRY(createVulkanEntity(
        &renderObjectManager, errorMessage, objects, device, imageManager, meshManager, _framesInFlight
    ));

    TRY(createVulkanEntity(&meshManager, errorMessage, device, commandManager));

    // Pipeline managers creation
    TRY(createVulkanEntity(&shaderProgramManager, errorMessage, logicalDevice));
    TRY(createVulkanEntity(&pipelineManager, errorMessage, logicalDevice, swapchain));
    TRY(createVulkanEntity(&frameGraph, errorMessage, meshManager, device.getQueryPool()));

    auto meshRenderPass = std::make_unique<MeshRenderPass>();
    TRY(meshRenderPass->create(
        "mesh_render", shaderProgramManager, pipelineManager, frameResources, renderObjectManager, errorMessage
    ));

    frameGraph.addPass(std::move(meshRenderPass));

    frameGraph.attachSwapchainOutput();

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

    VulkanFrameContext frameContext{};
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
