#include "VulkanRenderGraphBuilder.h"

#include "core/render/BindingSlots.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

Expected<void> VulkanRenderGraphBuilder::build() {

    vk::DescriptorSetLayoutCreateInfo emptyInfo{};
    VK_CREATE(_emptyDescriptorLayout, _context.device.getLogicalDevice().createDescriptorSetLayout(emptyInfo));

    TRY(allocateResources());

    // Build passes
    for (const auto& passDescriptor : _passDescriptors) {
        VulkanGraphicsPass* pass;
        TRY_ASSIGN(pass, allocatePass(passDescriptor));

        TRY_ASSIGN(pass->base().getShaderProgram(), _context.shaderProgramManager.load(passDescriptor.base.programPath));

        TRY(resolvePushConstantRanges(pass));

        TRY(_passFactory.createPass(pass, _context));

        TRY(resolveAttachments(pass));
        TRY(allocateDescriptors(&pass->base()));
        TRY(resolveDescriptorLayouts(pass));
        TRY(createPipeline(pass));
    }

    scheduleResourceTransitions();

    Logger::debug("Built render graph");

    // TODO: Add validation step

    return {};
}

Expected<VulkanGraphicsPass*> VulkanRenderGraphBuilder::allocatePass(const VulkanGraphicsPassDescriptor& descriptor) const {
    _context.renderGraph.addPass(std::make_unique<VulkanGraphicsPass>(descriptor));

    return Expected(_context.renderGraph.getPasses().back().get());
}


Expected<void> VulkanRenderGraphBuilder::allocateResources() const {
    for (const auto& resourceDescriptor : _resourceDescriptors) {

        switch (resourceDescriptor.type) {
            case VulkanPassResourceType::Transient:
                TRY(_context.renderResources.createResource(resourceDescriptor));
                break;

            case VulkanPassResourceType::SwapchainOutput: {

                VulkanPassResource swapchainOutput(resourceDescriptor);
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

Expected<void> VulkanRenderGraphBuilder::resolveAttachments(VulkanGraphicsPass* pass) const {
    const VulkanGraphicsPassDescriptor& passDescriptor = pass->getGraphicsPassDescriptor();

    // Reads
    for (const auto& readDescriptor : passDescriptor.base.readDescriptors) {
        _context.renderResources.addResourceReader(readDescriptor.name, &pass->base());
    }

    // Writes (color attachments)
    for (const auto& attachmentDescriptor : passDescriptor.base.writeDescriptors) {
        const VulkanPassResource* resource = _context.renderResources.getResource(attachmentDescriptor.name);

        if (!resource) {
            return VK_FAIL("Failed to resolve color resource \"" + attachmentDescriptor.name + "\".");
        }

        VulkanGraphicsPassAttachment attachment(attachmentDescriptor);
        attachment.setResource(resource);

        pass->addColorAttachment(attachment);
        _context.renderResources.addResourceWriter(attachmentDescriptor.name, &pass->base());
    }

    // Depth attachment
    auto depthDescriptor = passDescriptor.depthAttachmentDescriptor;

    if (!depthDescriptor.name.empty()) {
        const VulkanPassResource* resource = _context.renderResources.getResource(depthDescriptor.name);

        if (!resource) {
            return VK_FAIL("Failed to resolve depth resource \"" + depthDescriptor.name + "\".");
        }

        // Determine loadOp based on whether previously built passes write to the depth buffer
        const auto& writers      = _context.renderResources.getResourceWriters();
        const bool  wasWrittenTo = writers.contains(depthDescriptor.name) && !writers.at(depthDescriptor.name).empty();

        depthDescriptor.loadOp = wasWrittenTo ? vk::AttachmentLoadOp::eLoad : vk::AttachmentLoadOp::eClear;

        VulkanGraphicsPassAttachment depthAttachment(depthDescriptor);
        depthAttachment.setResource(resource);

        pass->setDepthAttachment(depthAttachment);

        _context.renderResources.addResourceWriter(depthDescriptor.name, &pass->base());
    }

    return {};
}

Expected<void> VulkanRenderGraphBuilder::allocateDescriptors(VulkanPass* pass) const {
    TRY(_context.renderResources.allocateDescriptors(pass));

    return {};
}

Expected<void> VulkanRenderGraphBuilder::resolveDescriptorLayouts(VulkanGraphicsPass* pass) const {
    auto& layouts = pass->base().getPipelineLayoutDescriptor().descriptorLayouts;
    layouts.resize(BindingSlots::MaterialData + 1);

    layouts[BindingSlots::FrameData]    = _context.frameResources.getDescriptorManager().getLayout();

    layouts[BindingSlots::ObjectData]   = _context.renderObjectManager.getDescriptorManager().getLayout();

    layouts[BindingSlots::CullingData]  = _context.frameCuller.getDescriptorManager().getLayout();

    layouts[BindingSlots::PassData]     = pass->base().getPassDescriptor().readDescriptors.empty()
        ? _emptyDescriptorLayout
        : pass->base().getDescriptorManager()->getLayout();

    layouts[BindingSlots::MaterialData] = _context.materialManager.getDescriptorManager().getLayout();

    return {};
}

Expected<void> VulkanRenderGraphBuilder::resolvePushConstantRanges(VulkanGraphicsPass* pass) {
    for (const auto& pushConstant : pass->base().getShaderProgram()->getPushConstants()) {
        pass->base().getPipelineLayoutDescriptor().pushConstantRanges.emplace(pushConstant);
    }

    return {};
}

Expected<void> VulkanRenderGraphBuilder::createPipeline(VulkanGraphicsPass* pass) const {
    VulkanGraphicsPipelineDescriptor descriptor{};

    descriptor.shaderStages = pass->base().getShaderProgram()->getStages();
    descriptor.passType     = pass->getGraphicsPassDescriptor().type;
    descriptor.layout       = pass->base().getPipelineLayoutDescriptor();

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

    pass->setGraphicsPipeline(pipeline);

    return {};
}

void VulkanRenderGraphBuilder::scheduleResourceTransitions() const {
    const auto& writers = _context.renderResources.getResourceWriters();
    const auto& readers = _context.renderResources.getResourceReaders();

    for (const auto& [resourceName, resource] : _context.renderResources.getResources()) {
        if (!resource || !resource->resolveImage()) continue;

        const bool isSwapchain = resource->descriptor.type == VulkanPassResourceType::SwapchainOutput;
        const bool isDepth     = VulkanImage::isDepthBuffer(resource->resolveImage()->getFormat());

        // Entry transitions for writer passes
        if (writers.contains(resourceName)) {
            const vk::ImageLayout entryLayout = isDepth
                ? vk::ImageLayout::eDepthStencilAttachmentOptimal
                : vk::ImageLayout::eColorAttachmentOptimal;

            for (VulkanPass* pass : writers.at(resourceName)) {
                if (pass) pass->addEntryTransition({resource.get(), entryLayout});
            }
        }

        // Entry transitions for reader passes
        if (readers.contains(resourceName)) {
            for (VulkanPass* pass : readers.at(resourceName)) {
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

            for (VulkanPass* pass : writers.at(resourceName)) {
                if (pass) pass->addExitTransition({resource.get(), exitLayout});
            }
        }
    }
}
