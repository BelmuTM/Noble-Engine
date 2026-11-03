#include "VulkanRenderGraph.h"

#include "draw/VulkanDrawBatchBuilder.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/pipeline/graphics/VulkanGraphicsPipeline.h"

#include "graphics/vulkan/rendergraph/record/VulkanGraphicsRecordContext.h"

Expected<void> VulkanRenderGraph::create() noexcept {
    return {};
}

void VulkanRenderGraph::destroy() noexcept {
    for (const auto& pass : _passes) {
        pass->destroy();
    }

    _passes.clear();
}

Expected<void> VulkanRenderGraph::execute(
    const vk::CommandBuffer commandBuffer, const VulkanFrameContext& frameContext
) const {
    commandBuffer.resetQueryPool(frameContext.queryPool, 0, 1);

    for (const auto& pass : _passes) {
        TRY(executePass(commandBuffer, frameContext, *pass));
    }

    return {};
}

namespace {

Expected<void> executePassTransitions(
    const vk::CommandBuffer commandBuffer, const VulkanPass::TransitionsVector& transitions
) {
    for (const auto& [resource, targetLayout] : transitions) {
        if (VulkanImage* resourceImage = resource->resolveImage()) {
            TRY(resourceImage->transitionLayout(commandBuffer, targetLayout));
        }
    }

    return {};
}

}

Expected<void> VulkanRenderGraph::executePass(
    const vk::CommandBuffer commandBuffer, const VulkanFrameContext& frameContext, const VulkanGraphicsPass& pass
) {

    VulkanPassBindings passBindings{
        pass.getGraphicsPipeline(),
        &pass.base().getPipelineLayoutDescriptor(),
        pass.base().getDescriptorSets()
    };

    VulkanGraphicsRecordContext recordContext(
        commandBuffer,
        frameContext,
        passBindings,
        frameContext.drawBatchBuilder->getDrawBatch(pass)
    );

    // Transition resources for current pass
    TRY(executePassTransitions(commandBuffer, pass.base().getEntryTransitions()));

    // Color attachments
    std::vector<vk::RenderingAttachmentInfo> colorAttachments{};
    for (const auto& colorAttachment : pass.getColorAttachments()) {
        colorAttachments.push_back(colorAttachment->getInfo());
    }

    // Rendering info
    vk::RenderingInfo renderingInfo{};
    renderingInfo
        .setRenderArea({{0, 0}, recordContext.extent()})
        .setLayerCount(1)
        .setColorAttachments(colorAttachments);

    // Depth attachment
    vk::RenderingAttachmentInfo depthAttachment{};
    if (const auto* depth = pass.getDepthAttachment()) {
        depthAttachment = depth->getInfo();
        renderingInfo.setPDepthAttachment(&depthAttachment);
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
    TRY(executePassTransitions(commandBuffer, pass.base().getExitTransitions()));

    return {};
}
