#include "VulkanRenderGraph.h"

#include "draw/VulkanDrawBatchBuilder.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/pipeline/graphics/VulkanGraphicsPipeline.h"

#include "graphics/vulkan/rendergraph/record/VulkanGraphicsRecordContext.h"

Expected<void> VulkanRenderGraph::create() noexcept {
    return {};
}

void VulkanRenderGraph::destroy() noexcept {
    _passes.clear();
}

Expected<void> VulkanRenderGraph::execute(
    const vk::CommandBuffer           commandBuffer,
    const VulkanFrameContext&         frameContext,
    const VulkanGraphResourceManager& resources
) const {
    commandBuffer.resetQueryPool(frameContext.queryPool, 0, 1);

    for (const auto& pass : _passes) {
        TRY(executePass(commandBuffer, frameContext, resources, *pass));
    }

    return {};
}

namespace {

void recordTransitions(
    const vk::CommandBuffer commandBuffer, const VulkanPass::TransitionsVector& transitions
) {
    std::vector<vk::ImageMemoryBarrier2> barriers{};
    barriers.reserve(transitions.size());

    for (const auto& [resource, src, dst] : transitions) {

        const VulkanImage* resourceImage = resource->resolveImage();
        if (!resourceImage) continue;

        // Specify which region of the image to transition
        vk::ImageSubresourceRange subresourceRange{};
        subresourceRange
            .setAspectMask(resourceImage->getAspectFlags())
            .setBaseMipLevel(0)
            .setLevelCount(vk::RemainingMipLevels)
            .setBaseArrayLayer(0)
            .setLayerCount(vk::RemainingArrayLayers);

        barriers.emplace_back(
            vk::ImageMemoryBarrier2{}
                .setSrcStageMask(src.stages)
                .setSrcAccessMask(src.access)
                .setDstStageMask(dst.stages)
                .setDstAccessMask(dst.access)
                .setOldLayout(src.layout)
                .setNewLayout(dst.layout)
                .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
                .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
                .setImage(resourceImage->handle())
                .setSubresourceRange(subresourceRange)
        );
    }

    if (barriers.empty()) return;

    vk::DependencyInfo dependencyInfo{};
    dependencyInfo.setImageMemoryBarriers(barriers);

    commandBuffer.pipelineBarrier2(dependencyInfo);
}

[[nodiscard]] vk::RenderingAttachmentInfo makeAttachmentInfo(
    const VulkanGraphResource& resource, const VulkanPassResourceUse& use
) {
    const auto [loadOp, storeOp] = use.attachment.value_or(VulkanAttachmentOps{});

    return vk::RenderingAttachmentInfo{}
        .setImageView(resource.resolveImage()->getImageView())
        .setImageLayout(VulkanGraphResourceState::stateForUsage(use.usage, {}).layout)
        .setLoadOp(loadOp)
        .setStoreOp(storeOp)
        .setClearValue(resource.descriptor.clearValue);
}

}

Expected<void> VulkanRenderGraph::executePass(
    vk::CommandBuffer                 commandBuffer,
    const VulkanFrameContext&         frameContext,
    const VulkanGraphResourceManager& resources,
    const VulkanGraphicsPass&         pass
) {

    VulkanPassBindings passBindings(
        pass.base().getPipelines(),
        pass.base().getDescriptorSets()
    );

    VulkanGraphicsRecordContext recordContext(
        commandBuffer,
        frameContext,
        passBindings,
        frameContext.drawBatchBuilder->getDrawBatch(pass)  // TODO: Think of a better way to pass the batches
    );

    // Transition resources for current pass
    recordTransitions(commandBuffer, pass.base().getEntryTransitions());

    // Attachments
    std::vector<vk::RenderingAttachmentInfo>   colorAttachments{};
    std::optional<vk::RenderingAttachmentInfo> depthAttachment{};

    for (const auto& use : pass.base().getPassDescriptor().resources) {

        if (!VulkanPassResourceUse::isAttachmentUsage(use.usage)) continue;

        const VulkanGraphResource* resource = resources.getResource(use.name);

        if (!resource || !resource->resolveImage()) {
            return VK_FAIL("Attachment resource \"" + use.name + "\" has no image.");
        }

        const vk::RenderingAttachmentInfo attachmentInfo = makeAttachmentInfo(*resource, use);

        if (VulkanPassResourceUse::isDepthUsage(use.usage)) {
            // Depth attachment
            depthAttachment = attachmentInfo;
        } else {
            // Color attachment
            colorAttachments.push_back(attachmentInfo);
        }
    }

    // Rendering info
    vk::RenderingInfo renderingInfo{};
    renderingInfo
        .setRenderArea({{0, 0}, recordContext.extent()})
        .setLayerCount(1)
        .setColorAttachments(colorAttachments);

    if (depthAttachment) {
        renderingInfo.setPDepthAttachment(&*depthAttachment);
    }

#ifdef VULKAN_DEBUG_UTILS
    VulkanDebugger::beginLabel(commandBuffer, *frameContext.dispatchLoader, pass.getGraphicsPassDescriptor().base.name);
#endif

    const VulkanGraphicsPassDescriptor& passDescriptor = pass.getGraphicsPassDescriptor();

    // Start rendering
    commandBuffer.beginRendering(renderingInfo);

    if (passDescriptor.countPrimitives) commandBuffer.beginQuery(frameContext.queryPool, 0, {});

    TRY(passDescriptor.record(recordContext));

    if (passDescriptor.countPrimitives) commandBuffer.endQuery(frameContext.queryPool, 0);

    // Stop rendering
    commandBuffer.endRendering();

#ifdef VULKAN_DEBUG_UTILS
    VulkanDebugger::endLabel(commandBuffer, *frameContext.dispatchLoader);
#endif

    // Transition resources for next pass
    recordTransitions(commandBuffer, pass.base().getExitTransitions());

    return {};
}
