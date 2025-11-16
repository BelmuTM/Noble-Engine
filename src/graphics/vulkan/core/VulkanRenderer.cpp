#include "VulkanRenderer.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/pipeline/rendergraph/VulkanRenderGraphBuilder.h"
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
    TRY(createVulkanEntity(&pipelineManager, errorMessage, logicalDevice));
    TRY(createVulkanEntity(&renderGraph, errorMessage, meshManager, frameResources, device.getQueryPool()));

    TRY(VulkanRenderGraphBuilder::buildPasses(
        renderGraph,
        shaderProgramManager,
        imageManager,
        frameResources,
        renderObjectManager,
        errorMessage
    ));

    renderGraph.attachSwapchainOutput(swapchain);

    TRY(VulkanRenderGraphBuilder::createPipelines(renderGraph, pipelineManager, errorMessage));

    shaderProgramManager.destroy();

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

    uint32_t imageIndex;
    if (!swapchainManager.acquireNextImage(imageIndex, currentFrame, errorMessage, discardLogging)) return;

    frameResources.update(currentFrame, imageIndex, camera);
    renderObjectManager.updateObjects();

    if (!recordCurrentCommandBuffer(imageIndex, errorMessage)) return;
    if (!submitCurrentCommandBuffer(imageIndex, errorMessage, discardLogging)) return;

    currentFrame = (currentFrame + 1) % _framesInFlight;

    // Queried drawn triangles count
    VK_CALL(context.getDevice().getLogicalDevice().getQueryPoolResults(
        context.getDevice().getQueryPool(),
        0, 1,
        sizeof(uint64_t),
        &primitiveCount,
        sizeof(uint64_t),
        vk::QueryResultFlagBits::eWait |
        vk::QueryResultFlagBits::e64
    ), errorMessage);

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
    const vk::Format swapchainFormat = swapchain.getFormat();
    const vk::Image& swapchainImage  = swapchain.getImages()[imageIndex];

    TRY(VulkanImageLayoutTransitions::transitionImageLayout(
        swapchainImage,
        swapchainFormat,
        commandBuffer,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        1,
        errorMessage
    ));

    renderGraph.execute(commandBuffer);

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
    const uint32_t imageIndex, std::string& errorMessage, bool& discardLogging
) {
    const vk::CommandBuffer& currentBuffer = commandManager.getCommandBuffers()[currentFrame];

    TRY(swapchainManager.submitCommandBuffer(currentBuffer, currentFrame, imageIndex, errorMessage, discardLogging));

    return true;
}
