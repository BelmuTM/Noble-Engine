#include "VulkanRenderGraph.h"

#include "graphics/vulkan/common/VulkanDebugger.h"
#include "graphics/vulkan/pipeline/VulkanGraphicsPipeline.h"
#include "graphics/vulkan/rendergraph/VulkanRenderResources.h"

#include "core/debug/Logger.h"

#include <ranges>

bool VulkanRenderGraph::create(const VulkanRenderGraphCreateContext& context, std::string& errorMessage) noexcept {
    _context = context;

    _context.dispatchLoader = vk::detail::DispatchLoaderDynamic(
        context.instance->handle(),         vkGetInstanceProcAddr,
        context.device->getLogicalDevice(), vkGetDeviceProcAddr
    );

    return true;
}

void VulkanRenderGraph::destroy() const noexcept {
    for (const auto& pass : _passes) {
        for (const auto& descriptorManager : pass->getDescriptorManagers() | std::views::values) {
            descriptorManager->destroy();
        }
    }
}

void VulkanRenderGraph::execute(const vk::CommandBuffer commandBuffer) const {
    std::string errorMessage;
    ScopeGuard guard{[&errorMessage] { Logger::error(errorMessage); }};

    commandBuffer.resetQueryPool(_context.queryPool, 0, 1);

    for (const auto& pass : _passes) {
        if (!executePass(commandBuffer, *pass.get(), errorMessage)) return;
    }

    guard.release();
}

namespace {

bool prepareColorAttachments(
    const vk::CommandBuffer                   commandBuffer,
    const VulkanRenderPass&                   pass,
    std::vector<vk::RenderingAttachmentInfo>& colorAttachments,
    std::string& errorMessage
) {
    for (const auto& colorAttachment : pass.getColorAttachments()) {
        auto& colorResource = colorAttachment->resource;

        VulkanImage* colorImage = colorResource.resolveImage();

        if (colorImage) {
            // Color attachment transition
            TRY(colorImage->transitionLayout(
                commandBuffer, errorMessage,
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

    return true;
}

bool prepareDepthAttachment(
    const vk::CommandBuffer      commandBuffer,
    const VulkanRenderPass&      pass,
    const VulkanRenderResources* resources,
    vk::RenderingAttachmentInfo& depthAttachment,
    std::string&                 errorMessage
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
    TRY(depthImage->transitionLayout(commandBuffer, errorMessage, targetDepthLayout));

    if (depthAttachmentPtr) {
        depthAttachment
            .setImageView(depthImage->getImageView())
            .setImageLayout(depthImage->getLayout())
            .setLoadOp(depthAttachmentPtr->loadOp)
            .setStoreOp(depthAttachmentPtr->storeOp)
            .setClearValue(depthAttachmentPtr->clearValue);
    }

    return true;
}

void executeDrawCalls(
    const vk::CommandBuffer                  commandBuffer,
    const VulkanRenderPass&                  pass,
    const VulkanFrameResources*              frame,
    const vk::Extent2D                       extent,
    const vk::detail::DispatchLoaderDynamic& dispatchLoader
) {
    const uint32_t frameIndex = frame->getFrameIndex();

    const VulkanShaderProgram* shaderProgram  = pass.getPipelineDescriptor().shaderProgram;
    const vk::PipelineLayout   pipelineLayout = pass.getPipeline()->getLayout();

    // Bind pipeline

    commandBuffer.bindPipeline(pass.getBindPoint(), pass.getPipeline()->handle());

    for (const auto& drawCall : pass._visibleDrawCalls) {
        const auto& draw = *drawCall;

#if defined(VULKAN_DEBUG_UTILS)
        std::string meshName = draw.getOwner() ? draw.getOwner()->object->getModel().name : "Mesh";
        VulkanDebugger::beginLabel(commandBuffer, dispatchLoader, meshName);
#endif

        // Bind descriptors

        // TO-DO: Use a pre-allocated temporary vector + std::span for scalability (owned by VulkanFrameResources).
        std::vector<vk::DescriptorSet> descriptorSets{};
        descriptorSets.push_back(frame->getDescriptors()->getSet(frameIndex));

        for (const auto& drawDescriptorSets : draw.getDescriptorSets()) {
            descriptorSets.push_back(drawDescriptorSets->getSet(frameIndex));
        }

        for (const auto& passDescriptorSets : pass.getDescriptorSets() | std::views::values) {
            descriptorSets.push_back(passDescriptorSets->getSet(frameIndex));
        }

        if (!descriptorSets.empty()) {
            commandBuffer.bindDescriptorSets(
                pass.getBindPoint(), pipelineLayout, 0, descriptorSets, nullptr
            );
        }

        // Draw mesh

        draw.record(commandBuffer, extent, pipelineLayout, shaderProgram);

#if defined(VULKAN_DEBUG_UTILS)
        VulkanDebugger::endLabel(commandBuffer, dispatchLoader);
#endif
    }
}

bool executePostPassTransitions(
    const vk::CommandBuffer commandBuffer,
    const VulkanRenderPass& pass,
    std::string&            errorMessage
) {
    for (const auto& [resource, targetLayout] : pass.getTransitions()) {
        VulkanImage* resourceImage = resource->resolveImage();

        TRY(resourceImage->transitionLayout(commandBuffer, errorMessage, targetLayout));
    }

    return true;
}

}

bool VulkanRenderGraph::executePass(
    const vk::CommandBuffer commandBuffer, const VulkanRenderPass& pass, std::string& errorMessage
) const {
    const vk::Extent2D extent = _context.swapchain->getExtent();

    const bool isMeshPass = pass.getType() == VulkanRenderPassType::MeshRender;

    if (pass.getBindPoint() == vk::PipelineBindPoint::eGraphics) {
        // Color attachments
        std::vector<vk::RenderingAttachmentInfo> colorAttachments{};
        TRY(prepareColorAttachments(commandBuffer, pass, colorAttachments, errorMessage));

        // Depth attachment
        vk::RenderingAttachmentInfo depthAttachment{};
        TRY(prepareDepthAttachment(commandBuffer, pass, _context.resources, depthAttachment, errorMessage));

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
        VulkanDebugger::beginLabel(commandBuffer, _context.dispatchLoader, pass.getName());
#endif

        commandBuffer.beginRendering(renderingInfo);

        if (isMeshPass)
            commandBuffer.beginQuery(_context.queryPool, 0, {});

        // Draw calls
        executeDrawCalls(commandBuffer, pass, _context.frame, extent, _context.dispatchLoader);

        if (isMeshPass)
            commandBuffer.endQuery(_context.queryPool, 0);

        // Stop rendering
        commandBuffer.endRendering();

#if defined(VULKAN_DEBUG_UTILS)
        VulkanDebugger::endLabel(commandBuffer, _context.dispatchLoader);
#endif

        // Transition resources for next pass
        TRY(executePostPassTransitions(commandBuffer, pass, errorMessage));
    }

    return true;
}
