#include "VulkanRenderGraphBuilder.h"

#include "core/render/BindingSlots.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

Expected<void> VulkanRenderGraphBuilder::build() {

    vk::DescriptorSetLayoutCreateInfo emptyInfo{};
    VK_CREATE(_emptyDescriptorLayout, _context.device.getLogicalDevice().createDescriptorSetLayout(emptyInfo));

    TRY(allocateResources());

    ResourceStates resourceStates;

    // Build passes
    for (const auto& passDescriptor : _passDescriptors) {

        VulkanGraphicsPass* pass;
        TRY_ASSIGN(pass, allocatePass(passDescriptor));

        VulkanShaderProgram* shaderProgram = nullptr;
        TRY_ASSIGN(shaderProgram, _context.shaderProgramManager.load(passDescriptor.base.programPath));

        VulkanPipelineLayoutDescriptor pipelineLayoutDescriptor{};

        TRY(resolvePushConstantRanges(shaderProgram, pipelineLayoutDescriptor));
        TRY(scheduleTransitions(pass->base(), resourceStates, vk::PipelineStageFlagBits2::eFragmentShader));

        const VulkanPassDescriptorBinding* descriptorBinding = nullptr;
        TRY_ASSIGN(descriptorBinding, _context.graphResources.allocateDescriptors(pass->base().getPassDescriptor()));

        pass->base().setDescriptorSets(descriptorBinding ? descriptorBinding->sets : nullptr);

        TRY(resolveDescriptorLayouts(pipelineLayoutDescriptor, descriptorBinding));

        TRY(createPipeline(pass, shaderProgram, pipelineLayoutDescriptor));
    }

    Logger::debug("Built render graph");

    // TODO: Add validation step

    return {};
}

Expected<VulkanGraphicsPass*> VulkanRenderGraphBuilder::allocatePass(const VulkanGraphicsPassDescriptor& descriptor) const {

    _context.renderGraph.addPass(std::make_unique<VulkanGraphicsPass>(descriptor));

    VulkanGraphicsPass* passPtr = _context.renderGraph.getPasses().back().get();
    passPtr->base().setIndex(_context.renderGraph.getPasses().size() - 1);

    return Expected(passPtr);
}


Expected<void> VulkanRenderGraphBuilder::allocateResources() const {

    for (const auto& resourceDescriptor : _resourceDescriptors) {

        switch (resourceDescriptor.type) {
            case VulkanGraphResourceType::Transient:
                TRY(_context.graphResources.createResource(resourceDescriptor));
                break;

            case VulkanGraphResourceType::SwapchainOutput: {

                VulkanGraphResource swapchainOutput(resourceDescriptor);
                swapchainOutput.setImageResolver(
                    [&swapchain = _context.swapchain, &frameResources = _context.frameResources] {
                        return swapchain.getImage(frameResources.getImageIndex());
                    }
                );

                _context.graphResources.addResource(swapchainOutput);
                break;
            }

            default:
                break;
        }
    }

    return {};
}

Expected<void> VulkanRenderGraphBuilder::scheduleTransitions(
    VulkanPass& pass, ResourceStates& resourceStates, const vk::PipelineStageFlags2 shaderStages
) const {
    const auto presentState = VulkanGraphResourceState::stateForUsage(VulkanGraphResourceUsage::Present, {});

    std::unordered_set<std::string> usedInPass{};

    for (const auto& [name, usage, attachment] : pass.getPassDescriptor().resources) {
        VulkanGraphResource* resource = _context.graphResources.getResource(name);

        if (!resource) {
            return VK_FAIL("Failed to resolve resource \"" + name + "\".");
        }

        if (!usedInPass.insert(name).second) {
            return VK_FAIL(
                "Pass \"" + pass.getPassDescriptor().name + "\" uses resource \"" + name + "\" more than once."
            );
        }

        // A read-only attachment can't be cleared
        if (
            usage == VulkanGraphResourceUsage::DepthReadOnly &&
            attachment.value_or(VulkanAttachmentOps{}).loadOp == vk::AttachmentLoadOp::eClear
        ) {
            return VK_FAIL("Read-only depth attachment \"" + name + "\" must declare loadOp = Load.");
        }

        const auto nextState    = VulkanGraphResourceState::stateForUsage(usage, shaderStages);
        auto&      currentState = resourceStates.try_emplace(resource).first->second;

        if (currentState.needsBarrierTo(nextState)) {
            pass.addEntryTransition({.resource = resource, .src = currentState, .dst = nextState});
        }

        // Swapchain images go back to the present layout at the end of every pass that uses them
        if (resource->descriptor.type == VulkanGraphResourceType::SwapchainOutput) {
            pass.addExitTransition({.resource = resource, .src = nextState, .dst = presentState});
            currentState = presentState;

        } else {
            currentState = nextState;
        }
    }

    return {};
}

Expected<void> VulkanRenderGraphBuilder::resolveDescriptorLayouts(
    VulkanPipelineLayoutDescriptor&    pipelineLayoutDescriptor,
    const VulkanPassDescriptorBinding* passDescriptorBinding
) const {

    auto& layouts = pipelineLayoutDescriptor.descriptorLayouts;

    layouts.resize(BindingSlots::MaterialData + 1);

    layouts[BindingSlots::FrameData]  = _context.frameResources.getDescriptorManager().getLayout();
    layouts[BindingSlots::ObjectData] = _context.drawBatchBuilder.getDescriptorManager().getLayout();

    layouts[BindingSlots::PassData] = passDescriptorBinding
        ? passDescriptorBinding->manager.getLayout()
        : _emptyDescriptorLayout;

    layouts[BindingSlots::MaterialData] = _context.materialManager.getDescriptorManager().getLayout();

    return {};
}

Expected<void> VulkanRenderGraphBuilder::resolvePushConstantRanges(
    const VulkanShaderProgram*      shaderProgram,
    VulkanPipelineLayoutDescriptor& layoutDescriptor
) {
    layoutDescriptor.pushConstantRanges = shaderProgram->getPushConstants();

    return {};
}

Expected<void> VulkanRenderGraphBuilder::createPipeline(
    VulkanGraphicsPass*                   pass,
    const VulkanShaderProgram*            shaderProgram,
    const VulkanPipelineLayoutDescriptor& layoutDescriptor
) const {

    VulkanGraphicsPipelineDescriptor descriptor{};

    descriptor.shaderStages = shaderProgram->getStages();
    descriptor.state        = pass->getGraphicsPassDescriptor().pipelineState;
    descriptor.layout       = layoutDescriptor;

    for (const auto& use : pass->base().getPassDescriptor().resources) {

        if (!VulkanPassResourceUse::isAttachmentUsage(use.usage)) continue;

        const vk::Format format = _context.graphResources.getResource(use.name)->resolveImage()->getFormat();

        if (VulkanPassResourceUse::isDepthUsage(use.usage)) {
            // Depth attachment format
            descriptor.depthAttachmentFormat = format;
            descriptor.depthWrite            = use.usage == VulkanGraphResourceUsage::DepthAttachment;

        } else {
            // Color attachment formats
            descriptor.colorAttachmentFormats.push_back(format);
        }
    }

    const VulkanGraphicsPipeline* pipeline = nullptr;
    TRY_ASSIGN(pipeline, _context.pipelineManager.createGraphicsPipeline(descriptor));

    pass->setGraphicsPipeline(pipeline);

    return {};
}
