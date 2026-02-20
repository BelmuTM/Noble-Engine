#include "VulkanRenderer.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/rendergraph/VulkanRenderGraphBuilder.h"

#include "core/debug/ErrorHandling.h"
#include "core/debug/Logger.h"

VulkanRenderer::VulkanRenderer(const uint32_t framesInFlight) : _framesInFlight(framesInFlight) {}

bool VulkanRenderer::init(
    Window&              window,
    const AssetManager&  assetManager,
    const ObjectManager& objectManager,
    std::string&         errorMessage
) {
    _window = &window;

    errorMessage = "Failed to init Vulkan renderer: no error message provided.";

    ScopeGuard guard{[this] { shutdown(); }};

    // Create context (instance, device, surface, swapchain)
    TRY_deprecated(createVulkanEntity(&context, errorMessage, window));

    const VulkanSurface& surface       = context.getSurface();
    const VulkanDevice&  device        = context.getDevice();
    const vk::Device&    logicalDevice = device.getLogicalDevice();

    VulkanSwapchain& swapchain = context.getSwapchain();

    // Resource managers creation
    TRY_deprecated(createVulkanEntity(&swapchainManager, errorMessage, window, surface, device, swapchain, _framesInFlight));
    TRY_deprecated(createVulkanEntity(&commandManager, errorMessage, device, _framesInFlight));
    TRY_deprecated(createVulkanEntity(&meshManager, errorMessage, device, commandManager));
    TRY_deprecated(createVulkanEntity(&imageManager, errorMessage, device, commandManager));
    TRY_deprecated(createVulkanEntity(&uniformBufferManager, errorMessage, device, _framesInFlight));
    TRY_deprecated(createVulkanEntity(&renderResources, errorMessage, device, swapchain, commandManager, _framesInFlight));

    TRY_deprecated(createVulkanEntity(
        &frameResources, errorMessage, device, swapchain, imageManager, uniformBufferManager, _framesInFlight
    ));

    TRY_deprecated(createVulkanEntity(
        &renderObjectManager, errorMessage, objectManager, assetManager, device, imageManager, meshManager, _framesInFlight
    ));

    TRY_deprecated(createVulkanEntity(&frameDraws, errorMessage, frameResources));

    // Pipeline creation
    TRY_deprecated(createVulkanEntity(&shaderProgramManager, errorMessage, logicalDevice));
    TRY_deprecated(createVulkanEntity(&pipelineManager, errorMessage, logicalDevice));

    // Render graph construction
    TRY_deprecated(createVulkanEntity(&renderGraph, errorMessage, VulkanRenderGraphCreateContext{
        &context.getInstance(),
        &device,
        &swapchain,
        &frameResources,
        &renderResources,
        &frameDraws,
        device.getQueryPool()
    }));

    const VulkanRenderGraphBuilderContext renderGraphBuilderContext{
        .renderGraph          = renderGraph,
        .device               = device,
        .swapchain            = swapchain,
        .commandManager       = commandManager,
        .meshManager          = meshManager,
        .imageManager         = imageManager,
        .frameResources       = frameResources,
        .renderResources      = renderResources,
        .renderObjectManager  = renderObjectManager,
        .shaderProgramManager = shaderProgramManager,
        .pipelineManager      = pipelineManager
    };

    const std::vector<VulkanRenderPassDescriptor> passes = {
        {"debug",       VulkanRenderPassType::Debug     },
        {"mesh_render", VulkanRenderPassType::MeshRender},
        {"composite_0", VulkanRenderPassType::Composite },
        {"composite_1", VulkanRenderPassType::Composite }
    };

    VulkanRenderPassFactory passFactory{};
    passFactory.registerPassTypes();

    const VulkanRenderGraphBuilder renderGraphBuilder(renderGraphBuilderContext, passFactory);
    TRY_deprecated(renderGraphBuilder.build(passes, errorMessage));

    TRY_deprecated(meshManager.fillBuffers(errorMessage));

    guard.release();

    return true;
}

void VulkanRenderer::shutdown() {
    if (context.getDevice().getLogicalDevice()) {
        VK_CALL_LOG(context.getDevice().getLogicalDevice().waitIdle(), Logger::Level::ERROR);
    }

    flushDeletionQueue();
}

void VulkanRenderer::drawFrame(const Camera& camera, const DebugState& debugState) {
    bool discardLogging = swapchainManager.isOutOfDate();
    std::string errorMessage;

    ScopeGuard guard{[&discardLogging, &errorMessage] {
        if (!discardLogging) Logger::error(errorMessage);
    }};

    if (!onFramebufferResize(errorMessage)) return;

    uint32_t imageIndex;
    if (!swapchainManager.acquireNextImage(imageIndex, currentFrame, errorMessage, discardLogging)) return;

    // Frame data update
    frameResources.update(currentFrame, imageIndex, camera, debugState);
    // Render objects update
    renderObjectManager.updateObjects();
    // Frustum culling
    frameDraws.cullDraws(renderGraph.getPasses());

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
    if (!_window || !_window->isFramebufferResized()) return true;

    if (context.getDevice().getLogicalDevice()) {
        VK_CALL_LOG(context.getDevice().getLogicalDevice().waitIdle(), Logger::Level::ERROR);
    }

    TRY_deprecated(swapchainManager.recreateSwapchain(errorMessage));
    TRY_deprecated(renderResources.recreate(renderGraph, errorMessage));

    _window->setFramebufferResized(false);

    return true;
}

bool VulkanRenderer::recordCommandBuffer(
    const vk::CommandBuffer commandBuffer, const uint32_t imageIndex, std::string& errorMessage
) {
    constexpr vk::CommandBufferBeginInfo beginInfo{};
    VK_TRY(commandBuffer.begin(beginInfo), errorMessage);

    VulkanImage* swapchainImage = context.getSwapchain().getImage(imageIndex);

    TRY_deprecated(swapchainImage->transitionLayout(
        commandBuffer, errorMessage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal
    ));

    renderGraph.execute(commandBuffer);

    TRY_deprecated(swapchainImage->transitionLayout(
        commandBuffer, errorMessage,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR
    ));

    VK_TRY(commandBuffer.end(), errorMessage);

    return true;
}

bool VulkanRenderer::recordCurrentCommandBuffer(const uint32_t imageIndex, std::string& errorMessage) {
    const vk::CommandBuffer& currentBuffer = commandManager.getCommandBuffers()[currentFrame];
    VK_CALL_LOG(currentBuffer.reset(), Logger::Level::ERROR);

    TRY_deprecated(recordCommandBuffer(currentBuffer, imageIndex, errorMessage));

    return true;
}

bool VulkanRenderer::submitCurrentCommandBuffer(
    const uint32_t imageIndex, std::string& errorMessage, bool& discardLogging
) {
    const vk::CommandBuffer& currentBuffer = commandManager.getCommandBuffers()[currentFrame];

    TRY_deprecated(swapchainManager.submitCommandBuffer(currentBuffer, currentFrame, imageIndex, errorMessage, discardLogging));

    return true;
}
