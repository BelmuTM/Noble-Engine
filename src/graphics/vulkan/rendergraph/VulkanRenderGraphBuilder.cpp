#include "VulkanRenderGraphBuilder.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

Expected<void> VulkanRenderGraphBuilder::build(const std::vector<VulkanRenderPassDescriptor>& passDescriptors) const {
    TRY(createPasses(passDescriptors));
    TRY(createColorBuffers());
    TRY(attachSwapchainOutput());
    TRY(allocateDescriptors());
    TRY(createPipelines());

    _context.renderResources.scheduleResourceTransitions();

    return {};
}

Expected<void> VulkanRenderGraphBuilder::createPasses(
    const std::vector<VulkanRenderPassDescriptor>& passDescriptors
) const {
    for (const auto& [path, type] : passDescriptors) {
        std::unique_ptr<VulkanRenderPass> pass;
        VK_TRY_ASSIGN(pass, _passFactory.createPass(path, type, _context));

        _context.renderGraph.addPass(std::move(pass));
    }

    return {};
}

Expected<void> VulkanRenderGraphBuilder::createColorBuffers() const {
    for (auto& pass : _context.renderGraph.getPasses()) {
        TRY(_context.renderResources.createColorBuffers(pass.get()));
    }

    return {};
}

Expected<void> VulkanRenderGraphBuilder::allocateDescriptors() const {
    for (const auto& pass : _context.renderGraph.getPasses()) {
        TRY(_context.renderResources.allocateDescriptors(pass.get()));
    }

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

Expected<void> VulkanRenderGraphBuilder::createPipelines() const {
    for (const auto& pass : _context.renderGraph.getPasses()) {
        VulkanGraphicsPipeline* pipeline = _context.pipelineManager.allocatePipeline();

        TRY(_context.pipelineManager.createGraphicsPipeline(pipeline, *pass));

        pass->setPipeline(pipeline);
    }

    return {};
}
