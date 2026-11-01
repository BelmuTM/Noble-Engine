#include "VulkanRenderer.h"

#include <iostream>

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/rendergraph/VulkanRenderGraphBuilder.h"

#include "graphics/vulkan/rendergraph/VulkanFrameContext.h"
#include "rendergraph/VulkanRecordContext.h"

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
        }
    );

    renderGraphBuilder
        .registerResource({"depthBuffer", vk::Format::eD32Sfloat})
        .registerResource({"albedoBuffer"})
        .registerResource({"normalBuffer"})
        .registerResource({"debugBuffer", vk::Format::eR32G32B32A32Sfloat})
        .registerResource({"compositeBuffer", vk::Format::eR16G16B16A16Sfloat})
        .registerResource({"swapchainOutput", vk::Format::eB8G8R8A8Srgb, VulkanPassResourceType::SwapchainOutput})

        .addPass({
            .base = {
                .name = "Gbuffer_Pass", .programPath = "mesh_render",
                .writes = {{"albedoBuffer"}, {"normalBuffer"}},
                .record = [](const VulkanRecordContext& ctx) -> Expected<void> {
                    ctx.bindPipeline();
                    ctx.bindGlobalDescriptors();
                    ctx.bindPassDescriptors();
                    ctx.setViewport();
                    ctx.setScissor();
                    std::cout << "meow gbuffers" << std::endl;
                    return {};
                }
            },
            .cullMode = VulkanGraphicsPassCullMode::Frustum,
            .depthAttachmentDescriptor = {"depthBuffer"},
            .countPrimitives = true
        })
        .addPass({
            .base = {
                .name = "Composite0_Pass", .programPath = "composite_0",
                .reads  = {{"depthBuffer"}, {"albedoBuffer"}, {"normalBuffer"}},
                .writes = {{"compositeBuffer"}},
                .record = [](const VulkanRecordContext& ctx) -> Expected<void> {
                    ctx.bindPipeline();
                    ctx.bindGlobalDescriptors();
                    ctx.bindPassDescriptors();
                    ctx.setViewport();
                    ctx.setScissor();
                    std::cout << "meow composite 0" << std::endl;
                    return {};
                }
            },
            .pipelineState = { .cullMode = vk::CullModeFlagBits::eNone, .meshVertexInput = false }
        })
        .addPass({
            .base = {
                .name = "Composite1_Pass", .programPath = "composite_1",
                .reads  = {{"depthBuffer"}, {"albedoBuffer"}, {"normalBuffer"}, {"debugBuffer"}, {"compositeBuffer"}},
                .writes = {{"swapchainOutput"}},
                .record = [](const VulkanRecordContext& ctx) -> Expected<void> {
                    ctx.bindPipeline();
                    ctx.bindGlobalDescriptors();
                    ctx.bindPassDescriptors();
                    ctx.setViewport();
                    ctx.setScissor();
                    std::cout << "meow composite 1" << std::endl;
                    return {};
                }
            },
            .pipelineState = { .cullMode = vk::CullModeFlagBits::eNone, .meshVertexInput = false }
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
    TRY(frameCuller.cull(renderObjectManager, uniforms));

    // Frame rendering context
    const VulkanFrameContext frameContext{
        .extent     = context.getSwapchain().getExtent(),
        .frameIndex = currentFrame,
        .globalSets = {
            frameResources.getDescriptorSets()->getSet(currentFrame),
            renderObjectManager.getDescriptorSets()->getSet(currentFrame),
            frameCuller.getDescriptorSets()->getSet(currentFrame),
        },
        .vertexBuffer   = meshManager.getVertexBuffer().handle(),
        .indexBuffer    = meshManager.getIndexBuffer().handle(),
        .queryPool      = context.getDevice().getQueryPool(),
        .dispatchLoader = &context.getDevice().getDispatchLoader()
    };

    // Command buffer record and submit
    const vk::CommandBuffer currentCommandBuffer = commandManager.getCommandBuffers()[currentFrame];

    TRY(commandManager.record(
        currentCommandBuffer,
        [this, &frameContext](const vk::CommandBuffer cmd) -> Expected<void> {
            return renderGraph.execute(cmd, frameContext);
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
