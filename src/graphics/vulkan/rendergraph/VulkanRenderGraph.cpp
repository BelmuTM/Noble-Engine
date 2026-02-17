#include "VulkanRenderGraph.h"

#include "VulkanRenderResources.h"
#include "graphics/vulkan/pipeline/VulkanGraphicsPipeline.h"

#include "core/debug/Logger.h"

#include "core/render/FrustumCuller.h"

#include <ranges>

bool VulkanRenderGraph::create(
    const VulkanSwapchain&   swapchain,
    const VulkanMeshManager& meshManager,
    VulkanFrameResources&    frame,
    VulkanRenderResources&   resources,
    const vk::QueryPool      queryPool,
    std::string&             errorMessage
) noexcept {
    _swapchain   = &swapchain;
    _meshManager = &meshManager;
    _frame       = &frame;
    _resources   = &resources;
    _queryPool   = queryPool;

    return true;
}

void VulkanRenderGraph::destroy() noexcept {
    for (const auto& pass : _passes) {
        for (const auto& descriptorManager : pass->getDescriptorManagers() | std::views::values) {
            descriptorManager->destroy();
        }
    }

    _swapchain   = nullptr;
    _meshManager = nullptr;
    _frame       = nullptr;
    _resources   = nullptr;
    _queryPool   = VK_NULL_HANDLE;
}

void VulkanRenderGraph::execute(const vk::CommandBuffer commandBuffer) const {
    std::string errorMessage;
    ScopeGuard guard{[&errorMessage] { Logger::error(errorMessage); }};

    commandBuffer.resetQueryPool(_queryPool, 0, 1);

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
    const vk::CommandBuffer     commandBuffer,
    const VulkanRenderPass&     pass,
    const VulkanMeshManager*    meshManager,
    const VulkanFrameResources* frame,
    const vk::Extent2D          extent
) {
    const uint32_t frameIndex = frame->getFrameIndex();

    const vk::Buffer& vertexBuffer = meshManager->getVertexBuffer().handle();
    const vk::Buffer& indexBuffer  = meshManager->getIndexBuffer().handle();

    const VulkanShaderProgram* shaderProgram = pass.getPipelineDescriptor().shaderProgram;

    const vk::PipelineLayout pipelineLayout = pass.getPipeline()->getLayout();

    // Bind pipeline
    commandBuffer.bindPipeline(pass.getBindPoint(), pass.getPipeline()->handle());

    for (const auto& drawCall : pass._visibleDrawCalls) {
        const auto& draw = *drawCall;

        /*---------------------------------------*/
        /*            Bind Resources             */
        /*---------------------------------------*/

        // Descriptors

        // TO-DO: Use a pre-allocated temporary vector + std::span for scalability (owned by VulkanFrameResources).
        std::vector<vk::DescriptorSet> descriptorSets{};
        descriptorSets.push_back(frame->getDescriptors()->getSet(frameIndex));

        for (const auto& drawDescriptorSets : draw.descriptorSets) {
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

        // Push constants

        if (auto* drawPushConstant = dynamic_cast<const VulkanDrawCallWithPushConstants*>(&draw)) {
            drawPushConstant->pushConstants(commandBuffer, pipelineLayout, shaderProgram);
        }

        /*---------------------------------------*/
        /*              Draw Meshes              */
        /*---------------------------------------*/

        commandBuffer.setViewport(0, draw.resolveViewport(extent));
        commandBuffer.setScissor(0, draw.resolveScissor(extent));

        if (!draw.mesh->isBufferless()) {
            const vk::DeviceSize vertexOffset = draw.mesh->getVertexOffset();
            const vk::DeviceSize indexOffset  = draw.mesh->getIndexOffset();

            commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer, &vertexOffset);
            commandBuffer.bindIndexBuffer(indexBuffer, indexOffset, vk::IndexType::eUint32);
            commandBuffer.drawIndexed(draw.mesh->getIndices().size(), 1, 0, 0, 0);
        } else {
            commandBuffer.draw(draw.mesh->getVertices().size(), 1, 0, 0);
        }
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
    const vk::Extent2D extent = _swapchain->getExtent();

    const bool isMeshPass = pass.getType() == VulkanRenderPassType::MeshRender;

    if (pass.getBindPoint() == vk::PipelineBindPoint::eGraphics) {
        // Color attachments
        std::vector<vk::RenderingAttachmentInfo> colorAttachments{};
        TRY(prepareColorAttachments(commandBuffer, pass, colorAttachments, errorMessage));

        // Depth attachment
        vk::RenderingAttachmentInfo depthAttachment{};
        TRY(prepareDepthAttachment(commandBuffer, pass, _resources, depthAttachment, errorMessage));

        // Rendering info
        vk::RenderingInfo renderingInfo{};
        renderingInfo
            .setRenderArea({{0, 0}, extent})
            .setLayerCount(1)
            .setColorAttachments(colorAttachments);

        if (pass.getDepthAttachment())
            renderingInfo.setPDepthAttachment(&depthAttachment);

        // Start rendering
        commandBuffer.beginRendering(renderingInfo);

        if (isMeshPass)
            commandBuffer.beginQuery(_queryPool, 0, {});

        // Draw calls
        executeDrawCalls(commandBuffer, pass, _meshManager, _frame, extent);

        if (isMeshPass)
            commandBuffer.endQuery(_queryPool, 0);

        // Stop rendering
        commandBuffer.endRendering();

        // Transition resources for next pass
        TRY(executePostPassTransitions(commandBuffer, pass, errorMessage));
    }

    return true;
}
