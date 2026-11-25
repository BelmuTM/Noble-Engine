#include "VulkanRenderer.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/rendergraph/VulkanRenderGraphBuilder.h"

#include "graphics/vulkan/rendergraph/VulkanFrameContext.h"
#include "graphics/vulkan/rendergraph/record/VulkanRecordContext.h"

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

    TRY(createVulkanEntity(&graphResources, device, swapchain, _framesInFlight));
    TRY(createVulkanEntity(&frameResources, device, imageManager, uniformBufferManager, _framesInFlight));

    TRY(createVulkanEntity(&materialManager, device, imageManager, _framesInFlight));

    TRY(createVulkanEntity(&renderObjectManager,
        VulkanRenderObjectCreateContext(
            &objectManager,
            &assetManager,
            &meshManager,
            &materialManager,
            &storageBufferManager,
            _framesInFlight
        )
    ));

    TRY(
        createVulkanEntity(
            &drawBatchBuilder, device, storageBufferManager, renderObjectManager.getObjectBuffer(), _framesInFlight
        )
    );

    // Pipeline creation
    TRY(createVulkanEntity(&shaderProgramManager, logicalDevice));
    TRY(createVulkanEntity(&pipelineManager, logicalDevice));

    VulkanRenderGraphBuilder renderGraphBuilder(
        VulkanRenderGraphBuilderContext(
            renderGraph,
            device,
            swapchain,
            commandManager,
            meshManager,
            imageManager,
            frameResources,
            graphResources,
            materialManager,
            renderObjectManager,
            drawBatchBuilder,
            shaderProgramManager,
            pipelineManager
        )
    );

    const auto recordGbuffer = [](VulkanRecordContext& ctx) -> Expected<void> {
        ctx.beginQuery();
        ctx.bindPipeline(0); ctx.bindGlobalDescriptors(); ctx.bindPassDescriptors();
        ctx.setViewport(); ctx.setScissor();
        ctx.drawBatches();
        ctx.endQuery();
        return {};
    };

    const auto recordDebug = [](VulkanRecordContext& ctx) -> Expected<void> {
        ctx.bindPipeline(0); ctx.bindGlobalDescriptors(); ctx.bindPassDescriptors();
        ctx.setViewport(); ctx.setScissor();
        ctx.drawBatches();
        return {};
    };

    const auto recordFullscreen = [](VulkanRecordContext& ctx) -> Expected<void> {
        ctx.bindPipeline(0); ctx.bindGlobalDescriptors(); ctx.bindPassDescriptors();
        ctx.setViewport(); ctx.setScissor();
        ctx.bindGeometryBuffers();
        ctx.draw(3);
        return {};
    };

    static constexpr VulkanAttachmentOps loadOps{.loadOp = vk::AttachmentLoadOp::eLoad};

    renderGraphBuilder
        .registerResource({.name = "depthBuffer", .format = vk::Format::eD32Sfloat})
        .registerResource({.name = "albedoBuffer"})
        .registerResource({.name = "normalBuffer"})
        .registerResource({.name = "debugBuffer", .format = vk::Format::eR8G8B8A8Unorm})
        .registerResource({.name = "compositeBuffer", .format = vk::Format::eR16G16B16A16Sfloat})
        .registerResource({.name = "swapchainOutput", .format = vk::Format::eB8G8R8A8Srgb, .type = VulkanGraphResourceType::SwapchainOutput})

        .registerPipeline({
            .name = "gbuffer",
            .programPath = "mesh_render"
        })
        .registerPipeline({
            .name = "debug",
            .programPath = "debug",
            .state = { .topology = vk::PrimitiveTopology::eLineList, .cullMode = vk::CullModeFlagBits::eNone }
        })
        .registerPipeline({
            .name = "composite_0",
            .programPath = "composite_0",
            .state = { .cullMode = vk::CullModeFlagBits::eNone, .meshVertexInput = false }
        })
        .registerPipeline({
            .name = "composite_1",
            .programPath = "composite_1",
            .state = { .cullMode = vk::CullModeFlagBits::eNone, .meshVertexInput = false }
        })

        .addPass({
            .name = "Gbuffer_Pass",
            .resources = {
                {.name = "albedoBuffer", .usage = VulkanGraphResourceUsage::ColorAttachment},
                {.name = "normalBuffer", .usage = VulkanGraphResourceUsage::ColorAttachment},
                {.name = "depthBuffer", .usage = VulkanGraphResourceUsage::DepthAttachment}
            },
            .pipelines = {"gbuffer"},
            .drawListId = drawBatchBuilder.registerDrawList({.drawFlags = DrawFlags::Bit::Opaque, .cullMode = CullMode::Frustum}),
            .record = recordGbuffer
        })
        .addPass({
            .name = "Debug_Pass",
            .resources = {
                {.name = "debugBuffer", .usage = VulkanGraphResourceUsage::ColorAttachment},
                {.name = "depthBuffer", .usage = VulkanGraphResourceUsage::DepthAttachment, .attachment = loadOps}
            },
            .pipelines = {"debug"},
            .drawListId = drawBatchBuilder.registerDrawList({.drawFlags = DrawFlags::Bit::Debug, .cullMode = CullMode::Frustum}),
            .record = recordDebug
        })
        .addPass({
            .name = "Composite0_Pass",
            .resources  = {
                {.name = "depthBuffer", .usage = VulkanGraphResourceUsage::SampledRead},
                {.name = "albedoBuffer", .usage = VulkanGraphResourceUsage::SampledRead},
                {.name = "normalBuffer", .usage = VulkanGraphResourceUsage::SampledRead},
                {.name = "compositeBuffer", .usage = VulkanGraphResourceUsage::ColorAttachment}
            },
            .pipelines = {"composite_0"},
            .record = recordFullscreen
        })
        .addPass({
            .name = "Composite1_Pass",
            .resources  = {
                {.name = "depthBuffer", .usage = VulkanGraphResourceUsage::SampledRead},
                {.name = "albedoBuffer", .usage = VulkanGraphResourceUsage::SampledRead},
                {.name = "normalBuffer", .usage = VulkanGraphResourceUsage::SampledRead},
                {.name = "debugBuffer", .usage = VulkanGraphResourceUsage::SampledRead},
                {.name = "compositeBuffer", .usage = VulkanGraphResourceUsage::SampledRead},
                {.name = "swapchainOutput", .usage = VulkanGraphResourceUsage::ColorAttachment}
            },
            .pipelines = {"composite_1"},
            .record = recordFullscreen
        });

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
    frameCuller.cull(renderObjectManager, uniforms);

    // Build draw commands
    TRY(
        drawBatchBuilder.build(
            renderObjectManager.getRenderItems(),
            frameCuller.getVisibleItems(),
            currentFrame
        )
    );

    // Frame rendering context
    const VulkanFrameContext frameContext{
        .extent     = context.getSwapchain().getExtent(),
        .frameIndex = currentFrame,
        .globalSets = {
            frameResources  .getDescriptorSets()->getSet(currentFrame),
            drawBatchBuilder.getDescriptorSets()->getSet(currentFrame)
        },
        .vertexBuffer     = meshManager.getVertexBuffer().handle(),
        .indexBuffer      = meshManager.getIndexBuffer().handle(),
        .queryPool        = context.getDevice().getQueryPool(),
        .dispatchLoader   = &context.getDevice().getDispatchLoader(),
        .drawBatchBuilder = &drawBatchBuilder
    };

    // Command buffer record and submit
    const vk::CommandBuffer currentCommandBuffer = commandManager.getCommandBuffers()[currentFrame];

    TRY(commandManager.record(
        currentCommandBuffer,
        [this, &frameContext](const vk::CommandBuffer cmd) -> Expected<void> {
            return renderGraph.execute(cmd, frameContext, graphResources);
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
    TRY(graphResources.recreate());

    _window->setFramebufferResized(false);

    return {};
}
