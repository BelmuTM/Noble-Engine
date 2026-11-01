#include "VulkanRenderGraph.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/pipeline/graphics/VulkanGraphicsPipeline.h"
#include "graphics/vulkan/rendergraph/resources/VulkanRenderResourceManager.h"

#include "draw/VulkanDrawBatchBuilder.h"

#include "graphics/vulkan/rendergraph/VulkanRecordContext.h"

Expected<void> VulkanRenderGraph::create(const VulkanRenderGraphCreateContext& context) noexcept {
    _context = context;

    _context.dispatchLoader = vk::detail::DispatchLoaderDynamic(
        context.instance->handle(),         vkGetInstanceProcAddr,
        context.device->getLogicalDevice(), vkGetDeviceProcAddr
    );

    return {};
}

void VulkanRenderGraph::destroy() noexcept {
    for (const auto& pass : _passes) {
        pass->destroy();
    }

    _passes.clear();
}

Expected<void> VulkanRenderGraph::execute(const vk::CommandBuffer commandBuffer) const {
    commandBuffer.resetQueryPool(_context.queryPool, 0, 1);

    for (const auto& pass : _passes) {
        TRY(executePass(commandBuffer, *pass));
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
    const vk::CommandBuffer commandBuffer, const VulkanGraphicsPass& pass
) const {
    VulkanRecordContext recordCtx(
        commandBuffer,
        _context.swapchain->getExtent(),
        _context.frame->getFrameIndex(),
        _context.frame,
        _context.renderObjectManager,
        _context.frameCuller,
        _context.dispatchLoader
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
        .setRenderArea({{0, 0}, recordCtx.extent()})
        .setLayerCount(1)
        .setColorAttachments(colorAttachments);

    // Depth attachment
    vk::RenderingAttachmentInfo depthAttachment{};
    if (const auto* depth = pass.getDepthAttachment()) {
        depthAttachment = depth->getInfo();
        renderingInfo.setPDepthAttachment(&depthAttachment);
    }

#ifdef VULKAN_DEBUG_UTILS
    VulkanDebugger::beginLabel(commandBuffer, _context.dispatchLoader, pass.getGraphicsPassDescriptor().base.name);
#endif

    const VulkanGraphicsPassDescriptor& passDescriptor = pass.getGraphicsPassDescriptor();

    // Start rendering
    commandBuffer.beginRendering(renderingInfo);

    if (passDescriptor.countPrimitives) commandBuffer.beginQuery(_context.queryPool, 0, {});

    TRY(passDescriptor.base.record(recordCtx));

    if (passDescriptor.countPrimitives) commandBuffer.endQuery(_context.queryPool, 0);

    // Stop rendering
    commandBuffer.endRendering();

#ifdef VULKAN_DEBUG_UTILS
    VulkanDebugger::endLabel(commandBuffer, _context.dispatchLoader);
#endif

    // Transition resources for next pass
    TRY(executePassTransitions(commandBuffer, pass.base().getExitTransitions()));

    return {};
}
