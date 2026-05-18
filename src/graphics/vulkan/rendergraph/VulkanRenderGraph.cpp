#include "VulkanRenderGraph.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/pipeline/VulkanGraphicsPipeline.h"
#include "graphics/vulkan/rendergraph/resources/VulkanRenderResources.h"

#include "draw/VulkanDrawBatchBuilder.h"

#include <ranges>

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

Expected<void> prepareColorAttachments(
    const vk::CommandBuffer                   commandBuffer,
    const VulkanRenderPass&                   pass,
    std::vector<vk::RenderingAttachmentInfo>& colorAttachments
) {
    for (const auto& colorAttachment : pass.getColorAttachments()) {
        auto& colorResource = colorAttachment->resource;

        if (VulkanImage* colorImage = colorResource.resolveImage()) {
            // Color attachment transition
            TRY(colorImage->transitionLayout(
                commandBuffer,
                vk::ImageLayout::eColorAttachmentOptimal
            ));

            colorAttachments.push_back(
                vk::RenderingAttachmentInfo{}
                    .setImageView(colorImage->getImageView())
                    .setImageLayout(colorImage->getLayout())
                    .setLoadOp(colorAttachment->loadOp)
                    .setStoreOp(colorAttachment->storeOp)
                    .setClearValue(colorAttachment->clearValue)
            );
        }
    }

    return {};
}

Expected<void> prepareDepthAttachment(
    const vk::CommandBuffer      commandBuffer,
    const VulkanRenderPass&      pass,
    const VulkanRenderResources* resources,
    vk::RenderingAttachmentInfo& depthAttachment
) {
    const VulkanRenderPassAttachment* depthAttachmentPtr = pass.getDepthAttachment();

    VulkanImage* depthImage = resources->getDepthBufferAttachment()->resource.image;

    vk::ImageLayout targetDepthLayout;

    if (depthAttachmentPtr) {
        // Pass writes depth
        targetDepthLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    } else {
        // Pass reads depth
        targetDepthLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    }

    // Depth image transition
    TRY(depthImage->transitionLayout(commandBuffer, targetDepthLayout));

    if (depthAttachmentPtr) {
        depthAttachment
            .setImageView(depthImage->getImageView())
            .setImageLayout(depthImage->getLayout())
            .setLoadOp(depthAttachmentPtr->loadOp)
            .setStoreOp(depthAttachmentPtr->storeOp)
            .setClearValue(depthAttachmentPtr->clearValue);
    }

    return {};
}

void executeDrawCalls(
    const vk::CommandBuffer                  commandBuffer,
    const VulkanRenderPass&                  pass,
    const VulkanFrameCuller*                 frameCuller,
    const VulkanFrameResources*              frame,
    const vk::Extent2D                       extent,
    const vk::detail::DispatchLoaderDynamic& dispatchLoader
) {
    const std::uint32_t frameIndex = frame->getFrameIndex();

    const VulkanShaderProgram* shaderProgram = pass.getPipelineDescriptor().shaderProgram;

    const VulkanGraphicsPipeline* pipeline          = pass.getPipeline();
    const vk::PipelineLayout&     pipelineLayout    = pass.getPipeline()->getLayout();
    const vk::PipelineBindPoint&  pipelineBindPoint = pass.getPipeline()->getBindPoint();

    // Bind pipeline

    commandBuffer.bindPipeline(pipelineBindPoint, pipeline->handle());

    // Build draw batches and update the indirection buffer
    const std::uint32_t indirectionOffset = frameCuller->getIndirectionOffset(&pass);

    VulkanDrawBatchBuilder batchBuilder;
    batchBuilder.build(frameCuller->getDrawCalls(&pass), indirectionOffset);

    frameCuller->getIndirectionBuffer()->updateArrayMemory(frameIndex, batchBuilder.getIndirectionData(), indirectionOffset);

    for (auto& [drawCall, firstInstance, instanceCount] : batchBuilder.getBuiltDrawBatches()) {
        auto& draw = *drawCall;

#if defined(VULKAN_DEBUG_UTILS)
        VulkanDebugger::beginLabel(commandBuffer, dispatchLoader, draw.getName());
#endif

        // Bind descriptor sets

        // TODO: Use a pre-allocated temporary vector + std::span for scalability (owned by VulkanFrameResources).
        std::vector<vk::DescriptorSet> descriptorSets{};
        descriptorSets.push_back(frame->getDescriptorSets()->getSet(frameIndex));

        for (const auto& drawDescriptorSets : draw.getDescriptorSets()) {
            descriptorSets.push_back(drawDescriptorSets->getSet(frameIndex));
        }

        for (const auto& passDescriptorSets : pass.getDescriptorSets() | std::views::values) {
            descriptorSets.push_back(passDescriptorSets->getSet(frameIndex));
        }

        if (!descriptorSets.empty()) {
            commandBuffer.bindDescriptorSets(pipelineBindPoint, pipelineLayout, 0, descriptorSets, nullptr);
        }

        // Draw mesh

        draw.record(commandBuffer, extent, pipelineLayout, shaderProgram, instanceCount, firstInstance);

#if defined(VULKAN_DEBUG_UTILS)
        VulkanDebugger::endLabel(commandBuffer, dispatchLoader);
#endif
    }
}

Expected<void> executePostPassTransitions(const vk::CommandBuffer commandBuffer, const VulkanRenderPass& pass) {
    for (const auto& [resource, targetLayout] : pass.getTransitions()) {
        VulkanImage* resourceImage = resource->resolveImage();

        TRY(resourceImage->transitionLayout(commandBuffer, targetLayout));
    }

    return {};
}

}

Expected<void> VulkanRenderGraph::executePass(
    const vk::CommandBuffer commandBuffer, const VulkanRenderPass& pass
) const {
    const vk::Extent2D extent = _context.swapchain->getExtent();

    const bool isMeshPass = pass.getPassDescriptor().type == VulkanRenderPassType::MeshRender;

    if (pass.getPipeline()->getBindPoint() == vk::PipelineBindPoint::eGraphics) {
        // Color attachments
        std::vector<vk::RenderingAttachmentInfo> colorAttachments{};
        TRY(prepareColorAttachments(commandBuffer, pass, colorAttachments));

        // Depth attachment
        vk::RenderingAttachmentInfo depthAttachment{};
        TRY(prepareDepthAttachment(commandBuffer, pass, _context.resources, depthAttachment));

        // Rendering info
        vk::RenderingInfo renderingInfo{};
        renderingInfo
            .setRenderArea({{0, 0}, extent})
            .setLayerCount(1)
            .setColorAttachments(colorAttachments);

        if (pass.getDepthAttachment())
            renderingInfo.setPDepthAttachment(&depthAttachment);

        // Start rendering
#if defined(VULKAN_DEBUG_UTILS)
        VulkanDebugger::beginLabel(commandBuffer, _context.dispatchLoader, pass.getPassDescriptor().name);
#endif

        commandBuffer.beginRendering(renderingInfo);

        if (isMeshPass)
            commandBuffer.beginQuery(_context.queryPool, 0, {});

        // Draw calls
        executeDrawCalls(commandBuffer, pass, _context.frameCuller, _context.frame, extent, _context.dispatchLoader);

        if (isMeshPass)
            commandBuffer.endQuery(_context.queryPool, 0);

        // Stop rendering
        commandBuffer.endRendering();

#if defined(VULKAN_DEBUG_UTILS)
        VulkanDebugger::endLabel(commandBuffer, _context.dispatchLoader);
#endif

        // Transition resources for next pass
        TRY(executePostPassTransitions(commandBuffer, pass));
    }

    return {};
}
