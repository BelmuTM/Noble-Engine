#include "VulkanRenderGraphBuilder.h"

#include "core/debug/ScopedTimer.h"
#include "core/render/BindingSlots.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

Expected<void> VulkanRenderGraphBuilder::build() {

    ScopedTimer renderGraphTimer("Building render graph", [](const std::string& name, auto duration) {
        Logger::info(name + " took " + std::to_string(duration.count()) + " ms");
    });

    renderGraphTimer.start();

    vk::DescriptorSetLayoutCreateInfo emptyInfo{};
    VK_CREATE(_emptyDescriptorLayout, _context.device.getLogicalDevice().createDescriptorSetLayout(emptyInfo));

    ResourceStates resourceStates;

    // Build passes
    for (const auto& passDescriptor : _passDescriptors) {

        VulkanPass* pass;
        TRY_ASSIGN(pass, allocatePass(passDescriptor));

        TRY(scheduleTransitions(pass, resourceStates, vk::PipelineStageFlagBits2::eFragmentShader));

        const VulkanPassDescriptorBinding* descriptorBinding = nullptr;
        TRY_ASSIGN(descriptorBinding, _context.graphResources.allocateDescriptors(pass->getPassDescriptor()));

        pass->setDescriptorSets(descriptorBinding ? descriptorBinding->sets : nullptr);

        TRY(linkPipelines(pass, descriptorBinding));
    }

    // TODO: Add validation step

    return {};
}

Expected<VulkanPass*> VulkanRenderGraphBuilder::allocatePass(
    const VulkanPassDescriptor& descriptor
) const {

    _context.renderGraph.addPass(std::make_unique<VulkanPass>(descriptor));
    return Expected(_context.renderGraph.getPasses().back().get());
}

Expected<void> VulkanRenderGraphBuilder::scheduleTransitions(
    VulkanPass* pass, ResourceStates& resourceStates, const vk::PipelineStageFlags2 shaderStages
) const {
    const auto presentState = VulkanGraphResourceState::stateForUsage(VulkanGraphResourceUsage::Present, {});

    std::unordered_set<VulkanGraphResourceId> usedInPass{};

    for (const auto& [resourceId, usage, attachment] : pass->getPassDescriptor().resources) {

        VulkanGraphResource* resource = _context.graphResources.getResource(resourceId);

        if (!resource) {
            return VK_FAIL("Failed to resolve resource \"" + resource->descriptor.name + "\".");
        }

        if (!usedInPass.insert(resourceId).second) {
            return VK_FAIL(
                "Pass \"" + pass->getPassDescriptor().name + "\" uses resource \"" + resource->descriptor.name + "\" more than once."
            );
        }

        // A read-only attachment can't be cleared
        if (
            usage == VulkanGraphResourceUsage::DepthReadOnly &&
            attachment.value_or(VulkanAttachmentOps{}).loadOp == vk::AttachmentLoadOp::eClear
        ) {
            return VK_FAIL("Read-only depth attachment \"" + resource->descriptor.name + "\" must declare loadOp = Load.");
        }

        const auto nextState    = VulkanGraphResourceState::stateForUsage(usage, shaderStages);
        auto&      currentState = resourceStates.try_emplace(resource).first->second;

        if (currentState.needsBarrierTo(nextState)) {
            pass->addEntryTransition({.resource = resource, .src = currentState, .dst = nextState});
        }

        // Swapchain images go back to the present layout at the end of every pass that uses them
        if (resource->descriptor.type == VulkanGraphResourceType::SwapchainOutput) {
            pass->addExitTransition({.resource = resource, .src = nextState, .dst = presentState});
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

Expected<void> VulkanRenderGraphBuilder::linkPipelines(
    VulkanPass* pass, const VulkanPassDescriptorBinding* binding
) const {

    VulkanGraphicsPipelineTarget target{};
    TRY(resolveDescriptorLayouts(target.layout, binding));

    for (const auto& use : pass->getPassDescriptor().resources) {

        if (!VulkanPassResourceUse::isAttachmentUsage(use.usage)) continue;

        const vk::Format format = _context.graphResources.getResource(use.resourceId)->resolveImage()->getFormat();

        if (VulkanPassResourceUse::isDepthUsage(use.usage)) {
            // Depth attachment format
            target.depthAttachmentFormat = format;
            target.depthWrite            = use.usage == VulkanGraphResourceUsage::DepthAttachment;

        } else {
            // Color attachment formats
            target.colorAttachmentFormats.push_back(format);
        }
    }

    for (const auto& pipelineId : pass->getPassDescriptor().pipelines) {

        const VulkanGraphicsPipeline* pipeline = nullptr;
        TRY_ASSIGN(pipeline, _context.pipelineManager.getOrCreate(pipelineId, target));

        pass->addPipeline(pipeline);
    }

    return {};
}
