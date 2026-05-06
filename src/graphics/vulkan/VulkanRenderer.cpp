#include "VulkanRenderer.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/rendergraph/VulkanRenderGraphBuilder.h"

VulkanRenderer::VulkanRenderer(const std::uint32_t framesInFlight) : _framesInFlight(framesInFlight) {}

Expected<void> VulkanRenderer::init(
    Window&              window,
    const AssetManager&  assetManager,
    const ObjectManager& objectManager
) {
    _window = &window;

    ScopeGuard guard{[this] { shutdown(); }};

    // Create context (instance, device, surface, swapchain)
    TRY(context.create(window));

    const VulkanSurface& surface       = context.getSurface();
    const VulkanDevice&  device        = context.getDevice();
    const vk::Device&    logicalDevice = device.getLogicalDevice();

    VulkanSwapchain& swapchain = context.getSwapchain();

    // Resource managers creation
    TRY(createVulkanEntity(&swapchainManager, window, surface, device, swapchain, _framesInFlight));
    TRY(createVulkanEntity(&commandManager, device, _framesInFlight));
    TRY(createVulkanEntity(&meshManager, device, commandManager));
    TRY(createVulkanEntity(&imageManager, device, commandManager));

    TRY(createVulkanEntity(&uniformBufferManager, device, _framesInFlight));
    TRY(createVulkanEntity(&storageBufferManager, device, _framesInFlight));

    TRY(createVulkanEntity(&renderResources, device, swapchain, commandManager, _framesInFlight));
    TRY(createVulkanEntity(&frameResources, device, imageManager, uniformBufferManager, _framesInFlight));

    TRY(createVulkanEntity(&materialManager, device, imageManager, _framesInFlight));

    TRY(createVulkanEntity(
        &renderObjectManager, VulkanRenderObjectCreateContext{
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
    TRY(createVulkanEntity(&shaderProgramManager, logicalDevice));
    TRY(createVulkanEntity(&pipelineManager, logicalDevice));

    // Render graph construction
    TRY(createVulkanEntity(&renderGraph, VulkanRenderGraphCreateContext{
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

    TRY(renderGraphBuilder.build(passes));

    TRY(meshManager.fillBuffers());

    guard.release();

    return {};
}

void VulkanRenderer::shutdown() {
    if (context.getDevice().getLogicalDevice()) {
        VK_FIRE_AND_FORGET(context.getDevice().getLogicalDevice().waitIdle());
    }

    flushDeletionQueue();
}

Expected<void> VulkanRenderer::drawFrame(const FrameUniforms& uniforms) {
    TRY(onFramebufferResize());

    VulkanSwapchain::SwapchainOp<uint32_t> imageAcquireResult;
    VK_TRY_ASSIGN(imageAcquireResult, swapchainManager.acquireNextImage(currentFrame));

    if (!imageAcquireResult.value.has_value()) {
        return {};
    }

    const uint32_t imageIndex = imageAcquireResult.value.value();

    // Frame data update
    frameResources.update(currentFrame, imageIndex, uniforms);
    // Render objects update
    renderObjectManager.updateObjects(currentFrame);
    // Frustum culling
    frameDraws.cullDraws(renderGraph.getPasses(), uniforms);

    TRY(recordCurrentCommandBuffer(imageIndex));
    TRY(submitCurrentCommandBuffer(imageIndex));

    currentFrame = (currentFrame + 1) % _framesInFlight;

    // Queried drawn triangles count
    VK_FIRE_AND_FORGET(context.getDevice().getLogicalDevice().getQueryPoolResults(
        context.getDevice().getQueryPool(),
        0, 1,
        sizeof(std::uint64_t),
        &primitiveCount,
        sizeof(std::uint64_t),
        vk::QueryResultFlagBits::eWait |
        vk::QueryResultFlagBits::e64
    ));

    return {};
}

Expected<void> VulkanRenderer::onFramebufferResize() {
    if (!_window || !_window->isFramebufferResized()) return {};

    if (context.getDevice().getLogicalDevice()) {
        VK_TRY(context.getDevice().getLogicalDevice().waitIdle());
    }

    TRY(swapchainManager.recreateSwapchain());
    TRY(renderResources.recreate(renderGraph));

    _window->setFramebufferResized(false);

    return {};
}

Expected<void> VulkanRenderer::recordCommandBuffer(
    const vk::CommandBuffer commandBuffer, const std::uint32_t imageIndex
) {
    constexpr vk::CommandBufferBeginInfo beginInfo{};
    VK_TRY(commandBuffer.begin(beginInfo));

    VulkanImage* swapchainImage = context.getSwapchain().getImage(imageIndex);

    TRY(swapchainImage->transitionLayout(
        commandBuffer,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal
    ));

    TRY(renderGraph.execute(commandBuffer));

    TRY(swapchainImage->transitionLayout(
        commandBuffer,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR
    ));

    VK_TRY(commandBuffer.end());

    return {};
}

Expected<void> VulkanRenderer::recordCurrentCommandBuffer(const std::uint32_t imageIndex) {
    const vk::CommandBuffer& currentBuffer = commandManager.getCommandBuffers()[currentFrame];
    VK_TRY(currentBuffer.reset());

    TRY(recordCommandBuffer(currentBuffer, imageIndex));

    return {};
}

Expected<void> VulkanRenderer::submitCurrentCommandBuffer(const std::uint32_t imageIndex) {
    const vk::CommandBuffer& currentBuffer = commandManager.getCommandBuffers()[currentFrame];

    TRY(swapchainManager.submitCommandBuffer(currentBuffer, currentFrame, imageIndex));

    return {};
}
