#include "VulkanRenderer.h"
#include "graphics/vulkan/common/VulkanDebugger.h"
#include "graphics/vulkan/resources/VulkanMesh.h"

#include "core/Engine.h"
#include "core/debug/Logger.h"
#include "core/debug/ErrorHandling.h"

#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

VulkanRenderer::~VulkanRenderer() {
    shutdown();
}

bool VulkanRenderer::init(Platform::Window& window) {
    _window = &window;

    std::string errorMessage = "Failed to create Vulkan renderer context: no error message provided";

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
    TRY(createVulkanEntity(&uniformBuffers, errorMessage, device, MAX_FRAMES_IN_FLIGHT));
    TRY(createVulkanEntity(&objectDescriptor, errorMessage, logicalDevice, MAX_FRAMES_IN_FLIGHT));

    const std::vector descriptorLayoutBindings = {
        vk::DescriptorSetLayoutBinding(
            0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics, nullptr
        ),
        vk::DescriptorSetLayoutBinding(
            1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr
        )
    };

     const std::vector descriptorPoolSizes = {
         vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT),
         vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT)
     };

    TRY(objectDescriptor.createSetLayout(descriptorLayoutBindings, errorMessage));

    VulkanShaderProgram program(logicalDevice);
    TRY(program.loadFromFiles({"meow.vert.spv", "meow.frag.spv"}, errorMessage));

    TRY(createVulkanEntity(&pipeline, errorMessage, logicalDevice, swapchain, objectDescriptor.getLayout(), program));

    TRY(objectDescriptor.createPool(descriptorPoolSizes, errorMessage));
    TRY(objectDescriptor.allocateSets(errorMessage));

    vk::DescriptorSetLayoutBinding uboBinding = {
        0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics, nullptr
    };

    uniformBuffers.bindToDescriptor(objectDescriptor, uboBinding.binding);

    TRY(createVulkanEntity(&meshMesh, errorMessage, device, commandManager));
    TRY(meshMesh.loadTextureFromFile("../../res/textures/mesh_mesh.png", errorMessage));

    vk::DescriptorSetLayoutBinding imageBinding = {
        1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr
    };

    meshMesh.bindToDescriptor(objectDescriptor, imageBinding.binding, MAX_FRAMES_IN_FLIGHT);

    VulkanMesh mesh{};
    const std::vector meshes = {mesh};

    TRY(createVulkanEntity(&meshManager, errorMessage, device, commandManager, meshes));

    DrawCall verticesDraw;
    verticesDraw.pipeline           = &pipeline;
    verticesDraw.mesh               = mesh;
    verticesDraw.descriptorResolver = [this](const FrameContext& frame) {
        return std::vector{ this->objectDescriptor.getDescriptorSets()[frame.frameIndex] };
    };

    FrameResource swapchainOutput = {
        {}, {}, {}, {}, vk::ImageLayout::eColorAttachmentOptimal,
        [](const FrameContext& frame) {
            return frame.swapchainImageView;
        }
    };

    FramePassAttachment swapchainAttachment{};
    swapchainAttachment.resource   = swapchainOutput;
    swapchainAttachment.loadOp     = vk::AttachmentLoadOp::eClear;
    swapchainAttachment.storeOp    = vk::AttachmentStoreOp::eStore;
    swapchainAttachment.clearValue = clearColor;

    FramePass mainPass;
    mainPass.name             = "MainPass";
    mainPass.bindPoint        = vk::PipelineBindPoint::eGraphics;
    mainPass.colorAttachments = { swapchainAttachment };
    mainPass.drawCalls        = { verticesDraw };

    frameGraph.addPass(mainPass);

    TRY(createVulkanEntity(&frameGraph, errorMessage, meshManager));

    guard.release();

    return true;
}

void VulkanRenderer::shutdown() {
    VK_CALL_LOG(context.getDevice().getLogicalDevice().waitIdle(), Logger::Level::ERROR);
    flushDeletionQueue();

    context.destroy();
}

void VulkanRenderer::drawFrame() {
    bool discardLogging = false;
    std::string errorMessage;

    ScopeGuard guard{[&discardLogging, &errorMessage] {
        if (!discardLogging) Logger::error(errorMessage);
    }};

    if (!swapchainManager.handleFramebufferResize(errorMessage)) return;

    const auto imageIndexOpt = swapchainManager.acquireNextImage(currentFrame, errorMessage, discardLogging);
    if (!imageIndexOpt) return;

    const uint32_t imageIndex = *imageIndexOpt;

    recordCurrentCommandBuffer(imageIndex);
    updateUniformBuffer();

    if (!swapchainManager.submitCommandBuffer(commandManager, currentFrame, imageIndex, errorMessage, discardLogging))
        return;

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    guard.release();
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
        errorMessage = "Failed to beging record for Vulkan command buffer: command buffer is null";
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
    frameContext.frameIndex         = currentFrame;
    frameContext.cmdBuffer          = commandBuffer;
    frameContext.extent             = context.getSwapchain().getExtent2D();
    frameContext.swapchainImageView = context.getSwapchain().getImageViews()[imageIndex];

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

// TO-DO: move this to UBO manager helper
void VulkanRenderer::updateUniformBuffer() {
    static auto startTime = std::chrono::high_resolution_clock::now();

    const auto  currentTime      = std::chrono::high_resolution_clock::now();
    const float frameTimeCounter = std::chrono::duration<float>(currentTime - startTime).count();

    const vk::Extent2D& extent      = context.getSwapchain().getExtent2D();
    const float         aspectRatio = static_cast<float>(extent.width) / static_cast<float>(extent.height);

    UniformBufferObject ubo{};
    ubo.model      = glm::rotate(glm::mat4(1.0f), frameTimeCounter * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view       = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 10.0f);

    ubo.projection[1][1] *= -1;

    memcpy(uniformBuffers.getBuffers()[currentFrame].getMappedPointer(), &ubo, sizeof(ubo));
}
