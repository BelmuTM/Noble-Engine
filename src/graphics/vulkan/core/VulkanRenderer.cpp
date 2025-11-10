#include "VulkanRenderer.h"

#include "graphics/vulkan/common/VulkanDebugger.h"
#include "graphics/vulkan/resources/mesh/VulkanMesh.h"

#include "core/Engine.h"
#include "core/debug/Logger.h"
#include "core/debug/ErrorHandling.h"

VulkanRenderer::~VulkanRenderer() {
    shutdown();
}

bool VulkanRenderer::init(Platform::Window& window, const std::vector<Object>& objects) {
    _window = &window;

    std::string errorMessage = "Failed to init Vulkan renderer: no error message provided";

    ScopeGuard guard{[this, &errorMessage] {
        shutdown();
        Engine::fatalExit(errorMessage);
    }};

    // Create Vulkan context
    TRY(context.create(window, errorMessage));

    // Create entities
    const VulkanDevice&    device              = context.getDevice();
    const VulkanSwapchain& swapchain           = context.getSwapchain();
    const vk::Device&      logicalDevice       = device.getLogicalDevice();
    const uint32_t         swapchainImageCount = swapchain.getImages().size();

    TRY(createVulkanEntity(&swapchainManager, errorMessage, context, window, MAX_FRAMES_IN_FLIGHT,
        swapchainImageCount));

    TRY(createVulkanEntity(&commandManager, errorMessage, device, MAX_FRAMES_IN_FLIGHT));
    TRY(createVulkanEntity(&pipelineManager, errorMessage, logicalDevice, swapchain));

    const std::vector descriptorLayoutBindingsFrame = {
        vk::DescriptorSetLayoutBinding(
            0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics, nullptr
        )
    };

    const std::vector descriptorPoolSizesFrame = {
        vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT)
    };

    TRY(createVulkanEntity(&descriptorManagerFrame, errorMessage, logicalDevice, MAX_FRAMES_IN_FLIGHT));
    TRY(descriptorManagerFrame.createSetLayout(descriptorLayoutBindingsFrame, errorMessage));
    TRY(descriptorManagerFrame.createPool(descriptorPoolSizesFrame, MAX_FRAMES_IN_FLIGHT, errorMessage));

    const std::vector descriptorLayoutBindingsObject = {
        vk::DescriptorSetLayoutBinding(
            0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr
        )
    };

    static constexpr uint32_t maxObjectSets = MAX_FRAMES_IN_FLIGHT * MAX_OBJECTS;

    const std::vector descriptorPoolSizesObject = {
        vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, maxObjectSets)
    };

    TRY(createVulkanEntity(&descriptorManagerObject, errorMessage, logicalDevice, MAX_FRAMES_IN_FLIGHT));
    TRY(descriptorManagerObject.createSetLayout(descriptorLayoutBindingsObject, errorMessage));
    TRY(descriptorManagerObject.createPool(descriptorPoolSizesObject, maxObjectSets, errorMessage));

    std::vector descriptorLayoutsMeshRender = {descriptorManagerFrame.getLayout(), descriptorManagerObject.getLayout()};
    std::vector descriptorLayoutsComposite  = {descriptorManagerFrame.getLayout()};

    TRY(createVulkanEntity(&imageManager, errorMessage, device, commandManager));
    TRY(createVulkanEntity(&uniformBufferManager, errorMessage, device, MAX_FRAMES_IN_FLIGHT));

    TRY(createVulkanEntity(
        &renderObjectManager, errorMessage, objects, device, descriptorManagerObject, imageManager, meshManager
    ));

    TRY(createVulkanEntity(&meshManager, errorMessage, device, commandManager, renderObjectManager.getMeshes()));

    TRY(uniformBufferManager.createBuffer(frameUBO, errorMessage));

    frameUBODescriptorSets = std::make_unique<VulkanDescriptorSets>(descriptorManagerFrame);
    TRY(frameUBODescriptorSets->allocate(errorMessage));
    frameUBODescriptorSets->bindPerFrameUBO(frameUBO, 0);

    VulkanShaderProgram composite(logicalDevice);
    TRY(composite.load("composite", true, errorMessage));

    TRY(pipelineManager.createGraphicsPipeline(pipelineComposite, descriptorLayoutsComposite, composite, errorMessage));

    VulkanShaderProgram meshRender(logicalDevice);
    TRY(meshRender.load("mesh_render", false, errorMessage));

    TRY(pipelineManager.createGraphicsPipeline(
        pipelineMeshRender, descriptorLayoutsMeshRender, objectDataGPUSize, meshRender, errorMessage
    ));

    FrameResource swapchainOutput{};
    swapchainOutput
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setResolveImageView([](const FrameContext& frame) { return frame.swapchainImageView; });

    FramePassAttachment swapchainAttachment{};
    swapchainAttachment
        .setResource(swapchainOutput)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(clearColor);

    TRY(imageManager.createDepthBuffer(depth, swapchain.getExtent(), errorMessage));

    FrameResource depthBuffer{};
    depthBuffer
        .setType(Buffer)
        .setImage(depth)
        .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
        .setResolveImageView([this](const FrameContext&) { return depth.getImageView(); });

    FramePassAttachment depthAttachment{};
    depthAttachment
        .setResource(depthBuffer)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(vk::ClearDepthStencilValue{1.0f, 0});

    /*
    TRY(imageManager.createColorBuffer(
        compositeOutput,
        swapchain.getExtent(),
        vk::Format::eR8G8B8A8Unorm,
        errorMessage
    ));

    FrameResource compositeBuffer{};
    compositeBuffer
        .setType(Buffer)
        .setImage(compositeOutput)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setResolveImageView([this](const FrameContext&) { return compositeOutput.getImageView(); });

    FramePassAttachment compositeAttachment{};
    compositeAttachment
        .setResource(compositeBuffer)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f});

    // TO-DO: Color attachment helper class
    FramePass compositePass;
    compositePass
        .setName("Composite_Pass")
        .setPipeline(&pipelineComposite)
        .setBindPoint(vk::PipelineBindPoint::eGraphics)
        //.addColorAttachment(compositeAttachment)
        .setDepthAttachment(depthAttachment);

    DrawCall fullscreenDraw;
    fullscreenDraw.setMesh(VulkanMesh::makeFullscreenTriangle());

    compositePass.addDrawCall(fullscreenDraw);
    */

    FramePass meshRenderPass;
    meshRenderPass
        .setName("MeshRender_Pass")
        .setPipeline(&pipelineMeshRender)
        .setBindPoint(vk::PipelineBindPoint::eGraphics)
        .addColorAttachment(swapchainAttachment)
        .setDepthAttachment(depthAttachment);

    for (const auto& renderObject : renderObjectManager.getRenderObjects()) {
        auto verticesDraw = std::make_unique<DrawCallPushConstant<ObjectDataGPU>>();
        verticesDraw->setMesh(*renderObject.mesh);

        verticesDraw->setDescriptorResolver(
            [&renderObject](const FrameContext& frame) {
                return std::vector{renderObject.descriptorSets->getSets().at(frame.frameIndex)};
            }
        );

        verticesDraw->setPushConstantResolver([&renderObject](const FrameContext&) { return renderObject.data; });

        meshRenderPass.addDrawCall(std::move(verticesDraw));
    }

    //frameGraph.addPass(compositePass);
    frameGraph.addPass(std::move(meshRenderPass));

    TRY(createVulkanEntity(&frameGraph, errorMessage, meshManager));

    guard.release();

    return true;
}

void VulkanRenderer::shutdown() {
    VK_CALL_LOG(context.getDevice().getLogicalDevice().waitIdle(), Logger::Level::ERROR);
    flushDeletionQueue();

    depth.destroy(context.getDevice());
    //compositeOutput.destroy(context.getDevice());

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

    recordCurrentCommandBuffer(imageIndex);

    frameUBO.update(currentFrame, context.getSwapchain(), camera);

    renderObjectManager.updateObjects();

    if (!swapchainManager.submitCommandBuffer(commandManager, currentFrame, imageIndex, errorMessage, discardLogging)) {
        return;
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    guard.release();
}

bool VulkanRenderer::onFramebufferResize(std::string& errorMessage) {
    if (!_window) return false;
    if (!_window->isFramebufferResized()) return true;

    VK_CALL_LOG(context.getDevice().getLogicalDevice().waitIdle(), Logger::Level::ERROR);

    depth.destroy(context.getDevice());

    TRY(swapchainManager.recreateSwapchain(errorMessage));

    TRY(imageManager.createDepthBuffer(depth, context.getSwapchain().getExtent(), errorMessage));

    _window->setFramebufferResized(false);

    return true;
}

void VulkanRenderer::transitionImageLayout(
    const vk::CommandBuffer       commandBuffer,
    const uint32_t                imageIndex,
    const vk::ImageLayout         oldLayout,
    const vk::ImageLayout         newLayout,
    const vk::AccessFlags2        srcAccessMask,
    const vk::AccessFlags2        dstAccessMask,
    const vk::PipelineStageFlags2 srcStageMask,
    const vk::PipelineStageFlags2 dstStageMask
) const {
    if (oldLayout == newLayout) return;

    // Specify which part of the image to transition
    vk::ImageSubresourceRange subresourceRange{};
    subresourceRange
        .setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setBaseMipLevel(0)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    // Define how the transition operates and what to change
    vk::ImageMemoryBarrier2 barrier{};
    barrier
        .setSrcStageMask(srcStageMask)
        .setSrcAccessMask(srcAccessMask)
        .setDstStageMask(dstStageMask)
        .setDstAccessMask(dstAccessMask)
        .setOldLayout(oldLayout)
        .setNewLayout(newLayout)
        .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored) // No ownership transfer to another queue family
        .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
        .setImage(context.getSwapchain().getImages()[imageIndex])
        .setSubresourceRange(subresourceRange);

    vk::DependencyInfo dependencyInfo{};
    dependencyInfo
        .setDependencyFlags({})
        .setImageMemoryBarriers(barrier);

    commandBuffer.pipelineBarrier2(dependencyInfo);
}

bool VulkanRenderer::beginCommandBuffer(const vk::CommandBuffer commandBuffer, std::string& errorMessage) {
    if (commandBuffer == vk::CommandBuffer{}) {
        errorMessage = "Failed to begin record for Vulkan command buffer: command buffer is null";
        return false;
    }

    constexpr vk::CommandBufferBeginInfo beginInfo{};
    VK_TRY_LOG(commandBuffer.begin(beginInfo), Logger::Level::ERROR);

    return true;
}

void VulkanRenderer::recordCommandBuffer(const vk::CommandBuffer commandBuffer, const uint32_t imageIndex) {
    std::string errorMessage;

    if (!beginCommandBuffer(commandBuffer, errorMessage)) {
        Logger::error(errorMessage);
        return;
    }

    transitionImageLayout(
        commandBuffer,
        imageIndex,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eTopOfPipe,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput
    );

    FrameContext frameContext{};
    frameContext
        .setFrameIndex(currentFrame)
        .setCommandBuffer(commandBuffer)
        .setSwapchainImageView(context.getSwapchain().getImageViews()[imageIndex])
        .setExtent(context.getSwapchain().getExtent());

    frameContext.frameDescriptors = frameUBODescriptorSets->getSets();

    frameGraph.execute(frameContext);

    transitionImageLayout(
        commandBuffer,
        imageIndex,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe
    );

    VK_CALL_LOG(commandBuffer.end(), Logger::Level::ERROR);
}

void VulkanRenderer::recordCurrentCommandBuffer(const uint32_t imageIndex) {
    const vk::CommandBuffer& currentBuffer = commandManager.getCommandBuffers()[currentFrame];
    VK_CALL_LOG(currentBuffer.reset(), Logger::Level::ERROR);

    recordCommandBuffer(currentBuffer, imageIndex);
}
