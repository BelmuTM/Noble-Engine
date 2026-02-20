#include "VulkanRenderGraphBuilder.h"

#include "core/debug/Logger.h"

bool VulkanRenderGraphBuilder::build(
    const std::vector<VulkanRenderPassDescriptor>& passDescriptors, std::string& errorMessage
) const {
    TRY_deprecated(createPasses(passDescriptors, errorMessage));

    TRY_deprecated(createColorBuffers(errorMessage));

    TRY_deprecated(attachSwapchainOutput(errorMessage));

    TRY_deprecated(allocateDescriptors(errorMessage));

    TRY_deprecated(createPipelines(errorMessage));

    _context.renderResources.scheduleResourceTransitions();

    return true;
}

bool VulkanRenderGraphBuilder::createPasses(
    const std::vector<VulkanRenderPassDescriptor>& passDescriptors, std::string& errorMessage
) const {
    for (const auto& [path, type] : passDescriptors) {
        Logger::debug(path);
        auto pass = _passFactory.createPass(path, type, _context, errorMessage);
        if (!pass) return false;

        _context.renderGraph.addPass(std::move(pass));
    }

    return true;
}

bool VulkanRenderGraphBuilder::createColorBuffers(std::string& errorMessage) const {
    for (auto& pass : _context.renderGraph.getPasses()) {
        TRY_deprecated(_context.renderResources.createColorBuffers(pass.get(), _context.frameResources, errorMessage));
    }

    return true;
}

bool VulkanRenderGraphBuilder::allocateDescriptors(std::string& errorMessage) const {
    for (const auto& pass : _context.renderGraph.getPasses()) {
        TRY_deprecated(_context.renderResources.allocateDescriptors(pass.get(), errorMessage));
    }

    return true;
}

bool VulkanRenderGraphBuilder::attachSwapchainOutput(std::string& errorMessage) const {
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
        errorMessage = "Failed to attach Vulkan swapchain output: last executing pass has no color attachments.";
        return false;
    }

    // Attach the swapchain output to the first declared color attachment of the last executing pass
    lastPass->getColorAttachments().at(0) = std::make_unique<VulkanRenderPassAttachment>(swapchainAttachment);

    return true;
}

bool VulkanRenderGraphBuilder::createPipelines(std::string& errorMessage) const {
    for (const auto& pass : _context.renderGraph.getPasses()) {
        VulkanGraphicsPipeline* pipeline = _context.pipelineManager.allocatePipeline();

        TRY_deprecated(_context.pipelineManager.createGraphicsPipeline(pipeline, *pass, errorMessage));

        pass->setPipeline(pipeline);
    }

    return true;
}
