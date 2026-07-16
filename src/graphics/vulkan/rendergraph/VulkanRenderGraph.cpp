#include "VulkanRenderGraph.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/pipeline/graphics/VulkanGraphicsPipeline.h"
#include "graphics/vulkan/rendergraph/resources/VulkanRenderResourceManager.h"

#include "draw/VulkanDrawBatchBuilder.h"

#include "core/render/BindingSlots.h"

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

void executeDrawCalls(
    const vk::CommandBuffer                  commandBuffer,
    const VulkanGraphicsPass&                pass,
    const vk::Extent2D                       extent,
    const vk::detail::DispatchLoaderDynamic& dispatchLoader,
    const VulkanFrameResources*              frame,
    const VulkanFrameCuller*                 frameCuller,
    const VulkanRenderObjectManager*         renderObjectManager
) {
    const std::uint32_t frameIndex = frame->getFrameIndex();

    const VulkanGraphicsPipeline* pipeline          = pass.getGraphicsPipeline();
    const vk::PipelineLayout&     pipelineLayout    = pipeline->getLayout();
    const vk::PipelineBindPoint&  pipelineBindPoint = VulkanGraphicsPipeline::getBindPoint();

    // Bind pipeline

    commandBuffer.bindPipeline(pipelineBindPoint, pipeline->handle());

    // Build draw batches and update the indirection buffer
    const std::uint32_t indirectionOffset = frameCuller->getIndirectionOffset(&pass);

    VulkanDrawBatchBuilder batchBuilder;
    batchBuilder.build(frameCuller->getDrawCalls(&pass), indirectionOffset);

    frameCuller->getIndirectionBuffer()->updateArrayMemory(frameIndex, batchBuilder.getIndirectionData(), indirectionOffset);

    const std::array fixedSets = {
        frame->getDescriptorSets()->getSet(frameIndex),               // slot 0: FrameData
        renderObjectManager->getDescriptorSets()->getSet(frameIndex), // slot 1: ObjectData
        frameCuller->getDescriptorSets()->getSet(frameIndex),         // slot 2: CullingData
    };
    commandBuffer.bindDescriptorSets(pipelineBindPoint, pipelineLayout, 0, fixedSets, nullptr);

    // slot 3: PassData
    if (const VulkanDescriptorSets* passDataSets = pass.base().getDescriptorSets()) {
        commandBuffer.bindDescriptorSets(
            pipelineBindPoint, pipelineLayout,
            BindingSlots::PassData,
            passDataSets->getSet(frameIndex),
            nullptr
        );
    }

    for (auto& [drawCall, firstInstance, instanceCount] : batchBuilder.getBuiltDrawBatches()) {
        auto& draw = *drawCall;

#ifdef VULKAN_DEBUG_UTILS
        VulkanDebugger::beginLabel(commandBuffer, dispatchLoader, draw.getName());
#endif

        // slot 4: MaterialData
        if (const VulkanMaterial* material = draw.getRenderMesh().material) {
            const vk::DescriptorSet materialSet = material->getDescriptorSets()->getSet(frameIndex);
            commandBuffer.bindDescriptorSets(
                pipelineBindPoint, pipelineLayout, BindingSlots::MaterialData, materialSet, nullptr
            );
        }

        // Draw mesh

        draw.record(commandBuffer, extent, pipelineLayout, instanceCount, firstInstance);

#ifdef VULKAN_DEBUG_UTILS
        VulkanDebugger::endLabel(commandBuffer, dispatchLoader);
#endif
    }
}

}

Expected<void> VulkanRenderGraph::executePass(
    const vk::CommandBuffer commandBuffer, const VulkanGraphicsPass& pass
) const {
    const vk::Extent2D extent = _context.swapchain->getExtent();

    const bool isMeshPass = pass.getGraphicsPassDescriptor().type == VulkanGraphicsPassType::MeshRender;

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
        .setRenderArea({{0, 0}, extent})
        .setLayerCount(1)
        .setColorAttachments(colorAttachments);

    // Depth attachment
    if (pass.getDepthAttachment()) {
        vk::RenderingAttachmentInfo depthAttachment = pass.getDepthAttachment()->getInfo();
        renderingInfo.setPDepthAttachment(&depthAttachment);
    }

    // Start rendering
#ifdef VULKAN_DEBUG_UTILS
    VulkanDebugger::beginLabel(commandBuffer, _context.dispatchLoader, pass.getGraphicsPassDescriptor().base.name);
#endif

    commandBuffer.beginRendering(renderingInfo);

    if (isMeshPass)
        commandBuffer.beginQuery(_context.queryPool, 0, {});

    // Draw calls
    executeDrawCalls(commandBuffer, pass, extent, _context.dispatchLoader, _context.frame, _context.frameCuller, _context.renderObjectManager);

    if (isMeshPass)
        commandBuffer.endQuery(_context.queryPool, 0);

    // Stop rendering
    commandBuffer.endRendering();

#ifdef VULKAN_DEBUG_UTILS
    VulkanDebugger::endLabel(commandBuffer, _context.dispatchLoader);
#endif

    // Transition resources for next pass
    TRY(executePassTransitions(commandBuffer, pass.base().getExitTransitions()));

    return {};
}
