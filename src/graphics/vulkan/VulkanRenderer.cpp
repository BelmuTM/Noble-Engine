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

    TRY(createVulkanEntity(&renderObjectManager,
        VulkanRenderObjectCreateContext{
            &objectManager,
            &assetManager,
            &device,
            &meshManager,
            &materialManager,
            &storageBufferManager,
            _framesInFlight
        }
    ));

    TRY(createVulkanEntity(&frameCuller, device, storageBufferManager, _framesInFlight));

    // Pipeline creation
    TRY(createVulkanEntity(&shaderProgramManager, logicalDevice));
    TRY(createVulkanEntity(&pipelineManager, logicalDevice));

    // Render graph construction
    TRY(createVulkanEntity(&renderGraph,
        VulkanRenderGraphCreateContext{
            &context.getInstance(),
            &device,
            &swapchain,
            &frameResources,
            &frameCuller,
            device.getQueryPool()
        }
    ));

    VulkanRenderPassFactory passFactory{};
    passFactory.registerPassTypes();

    VulkanRenderGraphBuilder renderGraphBuilder(
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
            frameCuller,
            shaderProgramManager,
            pipelineManager
        },
        passFactory
    );

    renderGraphBuilder
        .registerResource({"depthBuffer", vk::Format::eD32Sfloat})
        .registerResource({"albedoBuffer"})
        .registerResource({"normalBuffer"})
        .registerResource({"debugBuffer"})
        .registerResource({"compositeBuffer", vk::Format::eR16G16B16A16Sfloat})
        .registerResource({"swapchainOutput", vk::Format::eB8G8R8A8Srgb, VulkanRenderPassResourceType::SwapchainOutput})
        .addPass(
            {
                "Gbuffer_Pass", "mesh_render", VulkanRenderPassType::MeshRender, VulkanRenderPassCullMode::Frustum,
                {{"albedoBuffer"}, {"normalBuffer"}},
                {"depthBuffer", vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ClearDepthStencilValue{0.0f, 0}}
            }
        )
        .addPass(
            {
                "Debug_Pass", "debug", VulkanRenderPassType::Debug, VulkanRenderPassCullMode::None,
                {{"debugBuffer"}},
                {"depthBuffer", vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ClearDepthStencilValue{0.0f, 0}}
            }
        )
        .addPass(
            {
                "Composite0_Pass", "composite_0", VulkanRenderPassType::Composite, VulkanRenderPassCullMode::None,
                {{"compositeBuffer"}}
            }
        )
        .addPass(
            {
                "Composite1_Pass", "composite_1", VulkanRenderPassType::Composite, VulkanRenderPassCullMode::None,
                {{"swapchainOutput"}}
            }
        );

    TRY(renderGraphBuilder.build());

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
    TRY_ASSIGN(imageAcquireResult, swapchainManager.acquireNextImage(currentFrame));

    if (!imageAcquireResult.value.has_value()) {
        return {};
    }

    const uint32_t imageIndex = imageAcquireResult.value.value();

    // Frame data update
    frameResources.update(currentFrame, imageIndex, uniforms);
    // Render objects update
    renderObjectManager.updateObjects(currentFrame);
    // Frustum culling
    TRY(frameCuller.cull(renderGraph.getPasses(), uniforms));

    // Command buffer record and submit
    const vk::CommandBuffer currentCommandBuffer = commandManager.getCommandBuffers()[currentFrame];

    TRY(commandManager.record(
        currentCommandBuffer,
        [this](const vk::CommandBuffer cmd) -> Expected<void> {
            return renderGraph.execute(cmd);
        }
    ));

    TRY(swapchainManager.submitCommandBuffer(currentCommandBuffer, currentFrame, imageIndex));

    currentFrame = (currentFrame + 1) % _framesInFlight;

    // Queried drawn triangles count
    TRY(context.getDevice().getQueryPoolResults<std::uint64_t>(
        &primitiveCount, vk::QueryResultFlagBits::eWait | vk::QueryResultFlagBits::e64
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
