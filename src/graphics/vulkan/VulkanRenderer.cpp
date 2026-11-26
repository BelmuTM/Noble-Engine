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

    TRY(createVulkanEntity(&graphResources, device, _framesInFlight));
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

    const DrawListId opaqueDrawList = drawBatchBuilder.registerDrawList({.drawFlags = DrawFlags::Bit::Opaque, .cullMode = CullMode::Frustum});
    const DrawListId debugDrawList  = drawBatchBuilder.registerDrawList({.drawFlags = DrawFlags::Bit::Debug, .cullMode = CullMode::Frustum});

    const VulkanGraphResourceId depthBuffer = graphResources.registerResource({.name = "depthBuffer", .format = vk::Format::eD32Sfloat});
    const VulkanGraphResourceId albedoBuffer = graphResources.registerResource({.name = "albedoBuffer"});
    const VulkanGraphResourceId normalBuffer = graphResources.registerResource({.name = "normalBuffer"});
    const VulkanGraphResourceId debugBuffer = graphResources.registerResource({.name = "debugBuffer", .format = vk::Format::eR8G8B8A8Unorm});
    const VulkanGraphResourceId compositeBuffer = graphResources.registerResource({.name = "compositeBuffer", .format = vk::Format::eR16G16B16A16Sfloat});

    const VulkanGraphResourceId swapchainOutput = graphResources.registerExternalResource(
        {.name = "swapchainOutput", .format = vk::Format::eB8G8R8A8Srgb, .type = VulkanGraphResourceType::SwapchainOutput},
        [&] { return swapchain.getImage(frameResources.getImageIndex()); }
    );

    TRY(graphResources.allocateResources(swapchain.getExtent()));

    const auto recordGbuffer = [opaqueDrawList](VulkanRecordContext& ctx) -> Expected<void> {
        ctx.beginQuery();
        ctx.bindPipeline(0); ctx.bindGlobalDescriptors(); ctx.bindPassDescriptors();
        ctx.setViewport(); ctx.setScissor();
        ctx.drawBatches(opaqueDrawList);
        ctx.endQuery();
        return {};
    };

    const auto recordDebug = [debugDrawList](VulkanRecordContext& ctx) -> Expected<void> {
        ctx.bindPipeline(0); ctx.bindGlobalDescriptors(); ctx.bindPassDescriptors();
        ctx.setViewport(); ctx.setScissor();
        ctx.drawBatches(debugDrawList);
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

        .addPass(
            VulkanPassDescriptor("Gbuffer_Pass")
                .resource({.resourceId = albedoBuffer, .usage = VulkanGraphResourceUsage::ColorAttachment})
                .resource({.resourceId = normalBuffer, .usage = VulkanGraphResourceUsage::ColorAttachment})
                .resource({.resourceId = depthBuffer, .usage = VulkanGraphResourceUsage::DepthAttachment})
                .pipeline("gbuffer")
                .execute(recordGbuffer)
        )
        .addPass(
            VulkanPassDescriptor("Debug_Pass")
                .resource({.resourceId = debugBuffer, .usage = VulkanGraphResourceUsage::ColorAttachment})
                .resource({.resourceId = depthBuffer, .usage = VulkanGraphResourceUsage::DepthAttachment, .attachment = loadOps})
                .pipeline("debug")
                .execute(recordDebug)
        )
        .addPass(
            VulkanPassDescriptor("Composite0_Pass")
                .resource({.resourceId = depthBuffer, .usage = VulkanGraphResourceUsage::SampledRead})
                .resource({.resourceId = albedoBuffer, .usage = VulkanGraphResourceUsage::SampledRead})
                .resource({.resourceId = normalBuffer, .usage = VulkanGraphResourceUsage::SampledRead})
                .resource({.resourceId = compositeBuffer, .usage = VulkanGraphResourceUsage::ColorAttachment})
                .pipeline("composite_0")
                .execute(recordFullscreen)
        )
        .addPass(
            VulkanPassDescriptor("Composite1_Pass")
                .resource({.resourceId = depthBuffer, .usage = VulkanGraphResourceUsage::SampledRead})
                .resource({.resourceId = albedoBuffer, .usage = VulkanGraphResourceUsage::SampledRead})
                .resource({.resourceId = normalBuffer, .usage = VulkanGraphResourceUsage::SampledRead})
                .resource({.resourceId = debugBuffer, .usage = VulkanGraphResourceUsage::SampledRead})
                .resource({.resourceId = compositeBuffer, .usage = VulkanGraphResourceUsage::SampledRead})
                .resource({.resourceId = swapchainOutput, .usage = VulkanGraphResourceUsage::ColorAttachment})
                .pipeline("composite_1")
                .execute(recordFullscreen)
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
    TRY(graphResources.recreate(context.getSwapchain().getExtent()));

    _window->setFramebufferResized(false);

    return {};
}
