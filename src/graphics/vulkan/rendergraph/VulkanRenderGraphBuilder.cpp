#include "VulkanRenderGraphBuilder.h"

#include "passes/CompositePass.h"
#include "passes/DebugPass.h"
#include "passes/MeshRenderPass.h"

#include "core/debug/Logger.h"

bool VulkanRenderGraphBuilder::build(std::string& errorMessage) {
    _factories[VulkanRenderPassType::MeshRender] = &createPassFactory<MeshRenderPass, MeshRenderPassCreateContext>;
    _factories[VulkanRenderPassType::Debug]      = &createPassFactory<DebugPass,      DebugPassCreateContext>;
    _factories[VulkanRenderPassType::Composite]  = &createPassFactory<CompositePass,  CompositePassCreateContext>;

    TRY_deprecated(buildPasses(errorMessage));

    TRY_deprecated(createColorBuffers(errorMessage));

    TRY_deprecated(attachSwapchainOutput(errorMessage));

    TRY_deprecated(allocateDescriptors(errorMessage));

    TRY_deprecated(setupResourceTransitions(errorMessage));

    TRY_deprecated(createPipelines(errorMessage));

    return true;
}

bool VulkanRenderGraphBuilder::buildPasses(std::string& errorMessage) const {
    TRY_deprecated(createPass("debug", VulkanRenderPassType::Debug, errorMessage));
    TRY_deprecated(createPass("mesh_render", VulkanRenderPassType::MeshRender, errorMessage));
    TRY_deprecated(createPass("composite_0", VulkanRenderPassType::Composite, errorMessage));
    TRY_deprecated(createPass("composite_1", VulkanRenderPassType::Composite, errorMessage));

    return true;
}

bool VulkanRenderGraphBuilder::createPass(
    const std::string& path, const VulkanRenderPassType type, std::string& errorMessage
) const {
    const auto it = _factories.find(type);
    if (it == _factories.end()) {
        errorMessage = "Failed to create Vulkan render pass: no factory registered for pass type.";
        return false;
    }

    auto pass = it->second(path, _context, errorMessage);
    if (!pass) return false;

    _context.renderGraph.addPass(std::move(pass));

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

bool VulkanRenderGraphBuilder::setupResourceTransitions(std::string& errorMessage) const {
    for (const auto& [resourceName, writerPasses] : _context.renderResources.getResourceWriters()) {
        auto it = _context.renderResources.getResources().find(resourceName);
        if (it == _context.renderResources.getResources().end()) continue;

        VulkanRenderPassResource* resource = it->second.get();

        for (VulkanRenderPass* writerPass : writerPasses) {
            if (!writerPass) continue;
            writerPass->addTransition({resource, vk::ImageLayout::eShaderReadOnlyOptimal});
        }
    }

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
