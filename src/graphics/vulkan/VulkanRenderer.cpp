#include "VulkanRenderer.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/rendergraph/VulkanRenderGraphBuilder.h"

#include "core/debug/ErrorHandling.h"
#include "core/debug/Logger.h"

VulkanRenderer::VulkanRenderer(const std::uint32_t framesInFlight) : _framesInFlight(framesInFlight) {}

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
    TRY_BOOL(createVulkanEntity(&context, errorMessage, window));

    const VulkanSurface& surface       = context.getSurface();
    const VulkanDevice&  device        = context.getDevice();
    const vk::Device&    logicalDevice = device.getLogicalDevice();

    VulkanSwapchain& swapchain = context.getSwapchain();

    // Resource managers creation
    TRY_BOOL(createVulkanEntity(&swapchainManager, errorMessage, window, surface, device, swapchain, _framesInFlight));
    TRY_BOOL(createVulkanEntity(&commandManager, errorMessage, device, _framesInFlight));
    TRY_BOOL(createVulkanEntity(&meshManager, errorMessage, device, commandManager));
    TRY_BOOL(createVulkanEntity(&imageManager, errorMessage, device, commandManager));

    TRY_BOOL(createVulkanEntity(&uniformBufferManager, errorMessage, device, _framesInFlight));
    TRY_BOOL(createVulkanEntity(&storageBufferManager, errorMessage, device, _framesInFlight));

    TRY_BOOL(createVulkanEntity(&renderResources, errorMessage, device, swapchain, commandManager, _framesInFlight));

    TRY_BOOL(createVulkanEntity(
        &frameResources, errorMessage, device, imageManager, uniformBufferManager, _framesInFlight
    ));

    TRY_BOOL(createVulkanEntity(&materialManager, errorMessage, device, imageManager, _framesInFlight));

    TRY_BOOL(createVulkanEntity(
        &renderObjectManager, errorMessage, VulkanRenderObjectCreateContext{
            &objectManager,
            &assetManager,
            &device,
            &meshManager,
            &materialManager,
            &storageBufferManager,
            _framesInFlight
        }
    ));

    // Pipeline creation
    TRY_BOOL(createVulkanEntity(&shaderProgramManager, errorMessage, logicalDevice));
    TRY_BOOL(createVulkanEntity(&pipelineManager, errorMessage, logicalDevice));

    // Render graph construction
    TRY_BOOL(createVulkanEntity(&renderGraph, errorMessage, VulkanRenderGraphCreateContext{
        &context.getInstance(),
        &device,
        &swapchain,
        &frameResources,
        &renderResources,
        &frameDraws,
        device.getQueryPool()
    }));

    const std::vector<VulkanRenderPassDescriptor> passes = {
        {"mesh_render", VulkanRenderPassType::MeshRender},
        {"debug",       VulkanRenderPassType::Debug     },
        {"composite_0", VulkanRenderPassType::Composite },
        {"composite_1", VulkanRenderPassType::Composite }
    };

    VulkanRenderPassFactory passFactory{};
    passFactory.registerPassTypes();

    const VulkanRenderGraphBuilder renderGraphBuilder(
        VulkanRenderGraphBuilderContext{
            renderGraph,
            device,
            swapchain,
            commandManager,
            meshManager,
            imageManager,
            frameResources,
            renderResources,
            materialManager,
            renderObjectManager,
            shaderProgramManager,
            pipelineManager
        },
        passFactory
     );

    TRY_BOOL(renderGraphBuilder.build(passes, errorMessage));

    TRY_BOOL(meshManager.fillBuffers(errorMessage));

    guard.release();

    return true;
}

void VulkanRenderer::shutdown() {
    if (context.getDevice().getLogicalDevice()) {
        VK_CALL_LOG(context.getDevice().getLogicalDevice().waitIdle(), Logger::Level::ERROR);
    }

    flushDeletionQueue();
}

void VulkanRenderer::drawFrame(const FrameUniforms& uniforms) {
    bool discardLogging = swapchainManager.isOutOfDate();

    std::string errorMessage;

    ScopeGuard guard{[&discardLogging, &errorMessage] {
        if (!discardLogging) Logger::error(errorMessage);
    }};

    if (!onFramebufferResize(errorMessage)) return;

    std::uint32_t imageIndex;
    if (!swapchainManager.acquireNextImage(imageIndex, currentFrame, errorMessage, discardLogging)) return;

    // Frame data update
    frameResources.update(currentFrame, imageIndex, uniforms);
    // Render objects update
    renderObjectManager.updateObjects(currentFrame);
    // Frustum culling
    frameDraws.cullDraws(renderGraph.getPasses(), uniforms);

    if (!recordCurrentCommandBuffer(imageIndex, errorMessage)) return;
    if (!submitCurrentCommandBuffer(imageIndex, errorMessage, discardLogging)) return;

    currentFrame = (currentFrame + 1) % _framesInFlight;

    // Queried drawn triangles count
    VK_CALL(context.getDevice().getLogicalDevice().getQueryPoolResults(
        context.getDevice().getQueryPool(),
        0, 1,
        sizeof(std::uint64_t),
        &primitiveCount,
        sizeof(std::uint64_t),
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

    TRY_BOOL(swapchainManager.recreateSwapchain(errorMessage));
    TRY_BOOL(renderResources.recreate(renderGraph, errorMessage));

    _window->setFramebufferResized(false);

    return true;
}

bool VulkanRenderer::recordCommandBuffer(
    const vk::CommandBuffer commandBuffer, const std::uint32_t imageIndex, std::string& errorMessage
) {
    constexpr vk::CommandBufferBeginInfo beginInfo{};
    VK_TRY(commandBuffer.begin(beginInfo), errorMessage);

    VulkanImage* swapchainImage = context.getSwapchain().getImage(imageIndex);

    TRY_BOOL(swapchainImage->transitionLayout(
        commandBuffer, errorMessage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal
    ));

    renderGraph.execute(commandBuffer);

    TRY_BOOL(swapchainImage->transitionLayout(
        commandBuffer, errorMessage,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR
    ));

    VK_TRY(commandBuffer.end(), errorMessage);

    return true;
}

bool VulkanRenderer::recordCurrentCommandBuffer(const std::uint32_t imageIndex, std::string& errorMessage) {
    const vk::CommandBuffer& currentBuffer = commandManager.getCommandBuffers()[currentFrame];
    VK_CALL_LOG(currentBuffer.reset(), Logger::Level::ERROR);

    TRY_BOOL(recordCommandBuffer(currentBuffer, imageIndex, errorMessage));

    return true;
}

bool VulkanRenderer::submitCurrentCommandBuffer(
    const std::uint32_t imageIndex, std::string& errorMessage, bool& discardLogging
) {
    const vk::CommandBuffer& currentBuffer = commandManager.getCommandBuffers()[currentFrame];

    TRY_BOOL(swapchainManager.submitCommandBuffer(currentBuffer, currentFrame, imageIndex, errorMessage, discardLogging));

    return true;
}
