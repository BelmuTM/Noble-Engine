#include "VulkanRenderGraphBuilder.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

Expected<void> VulkanRenderGraphBuilder::build(const std::vector<VulkanRenderPassDescriptor>& passDescriptors) const {

    for (const auto& passDescriptor : passDescriptors) {
        VulkanRenderPass* pass;
        TRY_ASSIGN(pass, createPass(passDescriptor));

        TRY(createColorBuffers(pass));
    }

    TRY(attachSwapchainOutput());

    for (auto& pass : _context.renderGraph.getPasses()) {
        TRY(allocateDescriptors(pass.get()));
        TRY(createPipeline(pass.get()));
    }

    scheduleDepthLoadOps();
    scheduleResourceTransitions();

    return {};
}

Expected<VulkanRenderPass*> VulkanRenderGraphBuilder::createPass(const VulkanRenderPassDescriptor& passDescriptor) const {
    std::unique_ptr<VulkanRenderPass> pass;
    TRY_ASSIGN(pass, _passFactory.createPass(passDescriptor, _context));

    _context.renderGraph.addPass(std::move(pass));

    return Expected(_context.renderGraph.getPasses().back().get());
}

Expected<void> VulkanRenderGraphBuilder::createColorBuffers(VulkanRenderPass* pass) const {
    TRY(_context.renderResources.createColorBuffers(pass));

    return {};
}

Expected<void> VulkanRenderGraphBuilder::allocateDescriptors(VulkanRenderPass* pass) const {
    TRY(_context.renderResources.allocateDescriptors(pass));

    return {};
}

Expected<void> VulkanRenderGraphBuilder::createPipeline(VulkanRenderPass* pass) const {
    VulkanGraphicsPipeline* pipeline = _context.pipelineManager.allocatePipeline();

    TRY(_context.pipelineManager.createGraphicsPipeline(pipeline, *pass));

    pass->setPipeline(pipeline);

    return {};
}

Expected<void> VulkanRenderGraphBuilder::attachSwapchainOutput() const {
    static const std::string SWAPCHAIN_RESOURCE_NAME = "Swapchain_Output";

    const VulkanSwapchain& swapchain      = _context.swapchain;
    VulkanFrameResources&  frameResources = _context.frameResources;

    VulkanRenderPassResource swapchainOutput{};
    swapchainOutput
        .setName(SWAPCHAIN_RESOURCE_NAME)
        .setType(VulkanRenderPassResourceType::SwapchainOutput)
        .setImageResolver([&swapchain, &frameResources] {
            return swapchain.getImage(frameResources.getImageIndex());
        });

    VulkanRenderPassAttachment swapchainAttachment{};
    swapchainAttachment
        .setResource(swapchainOutput)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(defaultClearColor);

    VulkanRenderPass* lastPass = _context.renderGraph.getPasses().back().get();

    if (lastPass->getColorAttachments().empty()) {
        return VK_FAIL("Failed to attach swapchain output: last executing pass has no color attachments.");
    }

    // Attach the swapchain output to the first declared color attachment of the last executing pass
    lastPass->getColorAttachments().at(0) = std::make_unique<VulkanRenderPassAttachment>(swapchainAttachment);

    return {};
}

void VulkanRenderGraphBuilder::scheduleDepthLoadOps() const {
    bool depthWritten = false;

    const VulkanRenderPassAttachment* canonicalDepthAttachment = _context.renderResources.getDepthBufferAttachment();

    for (const auto& pass : _context.renderGraph.getPasses()) {
        if (!pass->getDepthAttachment()) continue;

        auto depthAttachment = std::make_unique<VulkanRenderPassAttachment>(*canonicalDepthAttachment);

        depthAttachment->setLoadOp(
            depthWritten ? vk::AttachmentLoadOp::eLoad
                         : vk::AttachmentLoadOp::eClear
        );

        pass->setDepthAttachment(std::move(depthAttachment));
        depthWritten = true;
    }
}

void VulkanRenderGraphBuilder::scheduleResourceTransitions() const {
    for (const auto& [resourceName, writerPasses] : _context.renderResources.getResourceWriters()) {
        auto it = _context.renderResources.getResources().find(resourceName);
        if (it == _context.renderResources.getResources().end()) continue;

        VulkanRenderPassResource* resource = it->second.get();

        for (VulkanRenderPass* writerPass : writerPasses) {
            if (!writerPass) continue;
            writerPass->addTransition({resource, vk::ImageLayout::eShaderReadOnlyOptimal});
        }
    }
}
