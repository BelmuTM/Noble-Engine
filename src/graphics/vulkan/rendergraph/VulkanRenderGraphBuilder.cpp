#include "VulkanRenderGraphBuilder.h"

#include "passes/CompositePass.h"
#include "passes/MeshRenderPass.h"

#include "core/debug/Logger.h"
#include "passes/DebugPass.h"

bool VulkanRenderGraphBuilder::build(std::string& errorMessage) const {
    TRY(buildPasses(errorMessage));

    TRY(createColorBuffers(errorMessage));

    TRY(attachSwapchainOutput(errorMessage));

    TRY(allocateDescriptors(errorMessage));

    TRY(setupResourceTransitions(errorMessage));

    TRY(createPipelines(errorMessage));

    return true;
}

bool VulkanRenderGraphBuilder::buildPasses(std::string& errorMessage) const {
    TRY(createPass("mesh_render", VulkanRenderPassType::MeshRender, errorMessage));
    TRY(createPass("debug", VulkanRenderPassType::Debug, errorMessage));
    TRY(createPass("composite_0", VulkanRenderPassType::Composite, errorMessage));
    TRY(createPass("composite_1", VulkanRenderPassType::Composite, errorMessage));

    return true;
}

// TO-DO: Factory function to register pass types and map them to creation contexts -> polymorphism.
bool VulkanRenderGraphBuilder::createPass(
    const std::string& path, const VulkanRenderPassType type, std::string& errorMessage
) const {
    switch (type) {
        case VulkanRenderPassType::MeshRender: {
            auto meshRenderPass = std::make_unique<MeshRenderPass>();

            TRY(meshRenderPass->create(path, MeshRenderPassCreateContext{
                _context.frameResources,
                _context.renderResources,
                _context.renderObjectManager,
                _context.shaderProgramManager,
            }, errorMessage));

            _context.renderGraph.addPass(std::move(meshRenderPass));

            break;
        }

        case VulkanRenderPassType::Debug: {
            auto debugPass = std::make_unique<DebugPass>();

            TRY(debugPass->create(path, DebugPassCreateContext{
                _context.device,
                _context.commandManager,
                _context.frameResources,
                _context.renderResources,
                _context.renderObjectManager,
                _context.shaderProgramManager,
            }, errorMessage));

            _context.renderGraph.addPass(std::move(debugPass));

            break;
        }

        case VulkanRenderPassType::Composite: {
            auto compositePass = std::make_unique<CompositePass>();

            TRY(compositePass->create(path, CompositePassCreateContext{
                _context.meshManager,
                _context.frameResources,
                _context.shaderProgramManager
            }, errorMessage));

            _context.renderGraph.addPass(std::move(compositePass));

            break;
        }

        default:
            break;
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

bool VulkanRenderGraphBuilder::createColorBuffers(std::string& errorMessage) const {
    for (auto& pass : _context.renderGraph.getPasses()) {
        TRY(_context.renderResources.createColorBuffers(pass.get(), _context.frameResources, errorMessage));
    }

    return true;
}

bool VulkanRenderGraphBuilder::allocateDescriptors(std::string& errorMessage) const {
    for (const auto& pass : _context.renderGraph.getPasses()) {
        TRY(_context.renderResources.allocateDescriptors(pass.get(), errorMessage));
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

        TRY(_context.pipelineManager.createGraphicsPipeline(pipeline, *pass, errorMessage));

        pass->setPipeline(pipeline);
    }

    return true;
}
