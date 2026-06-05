#include "VulkanRenderGraphBuilder.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include <ranges>

Expected<void> VulkanRenderGraphBuilder::build() const {

    TRY(allocateResources());

    // Build passes
    for (const auto& passDescriptor : _passDescriptors) {
        VulkanRenderPass* pass;
        TRY_ASSIGN(pass, createPass(passDescriptor));

        TRY(_context.shaderProgramManager.load(pass->getShaderProgram(), passDescriptor.programPath));

        TRY(resolveAttachments(pass));
        TRY(allocateDescriptors(pass));
        TRY(createPipeline(pass));
    }

    scheduleResourceTransitions();

    return {};
}

Expected<VulkanRenderPass*> VulkanRenderGraphBuilder::createPass(const VulkanRenderPassDescriptor& passDescriptor) const {
    std::unique_ptr<VulkanRenderPass> pass;
    TRY_ASSIGN(pass, _passFactory.createPass(passDescriptor, _context));

    _context.renderGraph.addPass(std::move(pass));

    return Expected(_context.renderGraph.getPasses().back().get());
}

Expected<void> VulkanRenderGraphBuilder::allocateResources() const {
    for (const auto& resourceDescriptor : _resourceDescriptors) {

        switch (resourceDescriptor.type) {
            case VulkanRenderPassResourceType::Transient:
                TRY(_context.renderResources.createResource(resourceDescriptor));
                break;

            case VulkanRenderPassResourceType::SwapchainOutput: {

                VulkanRenderPassResource swapchainOutput(resourceDescriptor);
                swapchainOutput.setImageResolver(
                    [&swapchain = _context.swapchain, &frameResources = _context.frameResources] {
                        return swapchain.getImage(frameResources.getImageIndex());
                    }
                );

                _context.renderResources.addResource(swapchainOutput);
                break;
            }

            default:
                break;
        }
    }

    return {};
}

Expected<void> VulkanRenderGraphBuilder::resolveAttachments(VulkanRenderPass* pass) const {
    // Reads
    for (const auto& readDescriptor : pass->getPassDescriptor().readDescriptors) {
        _context.renderResources.addResourceReader(readDescriptor.name, pass);
    }

    // Writes (color attachments)
    for (const auto& attachmentDescriptor : pass->getPassDescriptor().writeDescriptors) {
        const VulkanRenderPassResource* resource = _context.renderResources.getResource(attachmentDescriptor.name);

        if (!resource) {
            return VK_FAIL("Failed to resolve color resource \"" + attachmentDescriptor.name + "\".");
        }

        VulkanRenderPassAttachment attachment(attachmentDescriptor);
        attachment.setResource(resource);

        pass->addColorAttachment(attachment);
        _context.renderResources.addResourceWriter(attachmentDescriptor.name, pass);
    }

    // Depth attachment
    auto depthDescriptor = pass->getPassDescriptor().depthAttachmentDescriptor;

    if (!depthDescriptor.name.empty()) {
        const VulkanRenderPassResource* resource = _context.renderResources.getResource(depthDescriptor.name);

        if (!resource) {
            return VK_FAIL("Failed to resolve depth resource \"" + depthDescriptor.name + "\".");
        }

        // Determine loadOp based on whether previously built passes write to the depth buffer
        const auto& writers      = _context.renderResources.getResourceWriters();
        const bool  wasWrittenTo = writers.contains(depthDescriptor.name) && !writers.at(depthDescriptor.name).empty();

        depthDescriptor.loadOp = wasWrittenTo ? vk::AttachmentLoadOp::eLoad : vk::AttachmentLoadOp::eClear;

        VulkanRenderPassAttachment depthAttachment(depthDescriptor);
        depthAttachment.setResource(resource);

        pass->setDepthAttachment(depthAttachment);

        _context.renderResources.addResourceWriter(depthDescriptor.name, pass);
    }

    return {};
}

Expected<void> VulkanRenderGraphBuilder::allocateDescriptors(VulkanRenderPass* pass) const {
    TRY(_context.renderResources.allocateDescriptors(pass));

    return {};
}

Expected<void> VulkanRenderGraphBuilder::createPipeline(VulkanRenderPass* pass) const {
    VulkanGraphicsPipelineDescriptor descriptor{};

    descriptor.shaderStages = pass->getShaderProgram()->getStages();
    descriptor.passType     = pass->getPassDescriptor().type;

    // Descriptor layouts
    descriptor.layout = pass->getPipelineLayoutDescriptor();

    for (const auto& manager : pass->getDescriptorManagers() | std::views::values) {
        descriptor.layout.descriptorLayouts.push_back(manager->getLayout());
    }

    // Push constants
    for (const auto& [stageFlags, offset, size] : pass->getShaderProgram()->getPushConstants() | std::views::values) {
        descriptor.layout.pushConstantRanges.push_back({stageFlags, offset, size});
    }

    // Color attachment formats
    for (const auto& colorAttachment : pass->getColorAttachments()) {
        descriptor.colorAttachmentFormats.push_back(colorAttachment->resource->resolveImage()->getFormat());
    }

    // Depth attachment format
    if (pass->getDepthAttachment()) {
        descriptor.depthAttachmentFormat = pass->getDepthAttachment()->resource->resolveImage()->getFormat();
    }

    const VulkanGraphicsPipeline* pipeline = nullptr;
    TRY_ASSIGN(pipeline, _context.pipelineManager.createGraphicsPipeline(descriptor));

    pass->setPipeline(pipeline);

    return {};
}

void VulkanRenderGraphBuilder::scheduleResourceTransitions() const {
    const auto& writers = _context.renderResources.getResourceWriters();
    const auto& readers = _context.renderResources.getResourceReaders();

    for (const auto& [resourceName, resource] : _context.renderResources.getResources()) {
        if (!resource || !resource->resolveImage()) continue;

        const bool isSwapchain = resource->descriptor.type == VulkanRenderPassResourceType::SwapchainOutput;
        const bool isDepth     = VulkanImage::isDepthBuffer(resource->resolveImage()->getFormat());

        // Entry transitions for writer passes
        if (writers.contains(resourceName)) {
            const vk::ImageLayout entryLayout = isDepth
                ? vk::ImageLayout::eDepthStencilAttachmentOptimal
                : vk::ImageLayout::eColorAttachmentOptimal;

            for (VulkanRenderPass* pass : writers.at(resourceName)) {
                if (pass) pass->addEntryTransition({resource.get(), entryLayout});
            }
        }

        // Entry transitions for reader passes
        if (readers.contains(resourceName)) {
            for (VulkanRenderPass* pass : readers.at(resourceName)) {
                if (pass) pass->addEntryTransition({resource.get(), vk::ImageLayout::eShaderReadOnlyOptimal});
            }
        }

        // Exit transitions for writer passes
        if (writers.contains(resourceName)) {
            auto exitLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

            if (isSwapchain)
                exitLayout = vk::ImageLayout::ePresentSrcKHR;
            else if (isDepth && !readers.contains(resourceName))
                exitLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

            for (VulkanRenderPass* pass : writers.at(resourceName)) {
                if (pass) pass->addExitTransition({resource.get(), exitLayout});
            }
        }
    }
}
