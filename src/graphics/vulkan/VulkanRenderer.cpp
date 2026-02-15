#include "VulkanRenderer.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/pipeline/rendergraph/VulkanRenderGraphBuilder.h"

#include "core/debug/ErrorHandling.h"
#include "core/debug/Logger.h"
#include "core/render/FrustumCuller.h"
#include "pipeline/rendergraph/passes/MeshRenderPass.h"

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
    TRY(createVulkanEntity(&context, errorMessage, window));

    const VulkanSurface& surface       = context.getSurface();
    const VulkanDevice&  device        = context.getDevice();
    const vk::Device&    logicalDevice = device.getLogicalDevice();

    VulkanSwapchain& swapchain = context.getSwapchain();

    // Resource managers creation
    TRY(createVulkanEntity(&swapchainManager, errorMessage, window, surface, device, swapchain, _framesInFlight));
    TRY(createVulkanEntity(&commandManager, errorMessage, device, _framesInFlight));
    TRY(createVulkanEntity(&meshManager, errorMessage, device, commandManager));
    TRY(createVulkanEntity(&imageManager, errorMessage, device, commandManager));
    TRY(createVulkanEntity(&uniformBufferManager, errorMessage, device, _framesInFlight));
    TRY(createVulkanEntity(&renderResources, errorMessage, device, swapchain, commandManager, _framesInFlight));

    TRY(createVulkanEntity(
        &frameResources, errorMessage, device, swapchain, imageManager, uniformBufferManager, _framesInFlight
    ));

    TRY(createVulkanEntity(
        &renderObjectManager, errorMessage, objectManager, assetManager, device, imageManager, meshManager, _framesInFlight
    ));

    // Pipeline managers creation
    TRY(createVulkanEntity(&shaderProgramManager, errorMessage, logicalDevice));
    TRY(createVulkanEntity(&pipelineManager, errorMessage, logicalDevice));
    TRY(createVulkanEntity(
        &renderGraph, errorMessage, swapchain, meshManager, frameResources, renderResources, device.getQueryPool()
    ));

    const VulkanRenderGraphBuilderContext renderGraphBuilderContext{
        .renderGraph          = renderGraph,
        .meshManager          = meshManager,
        .imageManager         = imageManager,
        .frameResources       = frameResources,
        .renderResources      = renderResources,
        .renderObjectManager  = renderObjectManager,
        .shaderProgramManager = shaderProgramManager,
        .pipelineManager      = pipelineManager,
        .swapchain            = swapchain
    };

    const VulkanRenderGraphBuilder renderGraphBuilder(renderGraphBuilderContext);

    TRY(renderGraphBuilder.build(errorMessage));

    shaderProgramManager.destroy();

    TRY(meshManager.fillBuffers(errorMessage));

    guard.release();

    return true;
}

void VulkanRenderer::shutdown() {
    if (context.getDevice().getLogicalDevice()) {
        VK_CALL_LOG(context.getDevice().getLogicalDevice().waitIdle(), Logger::Level::ERROR);
    }

    flushDeletionQueue();
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

    const FrameUniforms& uniforms = frameResources.getUniforms();

    const glm::mat4 viewProjectionMatrix = uniforms.projectionMatrix * uniforms.viewMatrix;

    const std::array<Math::Plane, 6>& frustumPlanes = FrustumCuller::getFrustumPlanes(viewProjectionMatrix);

    for (const auto& pass : renderGraph.getPasses()) {
        pass->_visibleDrawCalls.clear();
        pass->_visibleDrawCalls.reserve(pass->getDrawCalls().size());

        for (const auto& drawCall : pass->getDrawCalls()) {
            bool visible = true;

            if (dynamic_cast<MeshRenderPass*>(pass.get())) {
                if (!drawCall->owner) continue;

                Math::AABB worldAABB = Math::AABB::transform(
                    drawCall->mesh->getAABB(), drawCall->owner->data.modelMatrix
                );

                //visible = FrustumCuller::testVisibility(worldAABB, frustumPlanes);
            }

            if (visible) {
                pass->_visibleDrawCalls.push_back(drawCall.get());
            }
        }
    }

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

    TRY(swapchainManager.recreateSwapchain(errorMessage));
    TRY(renderResources.recreate(renderGraph, errorMessage));

    _window->setFramebufferResized(false);

    return true;
}

bool VulkanRenderer::recordCommandBuffer(
    const vk::CommandBuffer commandBuffer, const uint32_t imageIndex, std::string& errorMessage
) {
    constexpr vk::CommandBufferBeginInfo beginInfo{};
    VK_TRY(commandBuffer.begin(beginInfo), errorMessage);

    VulkanImage* swapchainImage = context.getSwapchain().getImage(imageIndex);

    TRY(swapchainImage->transitionLayout(
        commandBuffer, errorMessage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal
    ));

    renderGraph.execute(commandBuffer);

    TRY(swapchainImage->transitionLayout(
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
