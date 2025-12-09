#include "VulkanRenderGraph.h"

#include "VulkanRenderResources.h"
#include "graphics/vulkan/resources/images/VulkanImageLayoutTransitions.h"

#include "core/debug/Logger.h"

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
        if (!executePass(commandBuffer, pass.get(), errorMessage)) return;
    }

    guard.release();
}

bool VulkanRenderGraph::executePass(
    const vk::CommandBuffer commandBuffer, const VulkanRenderPass* pass, std::string& errorMessage
) const {
    const uint32_t frameIndex = _frame->getFrameIndex();

    const vk::Extent2D extent = _swapchain->getExtent();

    const vk::Buffer& vertexBuffer = _meshManager->getVertexBuffer();
    const vk::Buffer& indexBuffer  = _meshManager->getIndexBuffer();

    const VulkanShaderProgram* shaderProgram = pass->getPipelineDescriptor().shaderProgram;

    const bool isFullScreenPass = shaderProgram->isFullscreen();

    if (pass->getBindPoint() == vk::PipelineBindPoint::eGraphics) {
        /*---------------------------------------*/
        /*     Rendering Info & Attachments      */
        /*---------------------------------------*/

        // Attach color buffers

        std::vector<vk::RenderingAttachmentInfo> colorAttachmentsInfo{};

        for (const auto& colorAttachment : pass->getColorAttachments()) {
            auto& colorResource = colorAttachment->resource;

            VulkanImage* colorImage = colorResource.resolveImage();

            if (colorImage) {
                TRY(colorImage->transitionLayout(
                    commandBuffer, errorMessage,
                    vk::ImageLayout::eColorAttachmentOptimal
                ));

                colorAttachmentsInfo.push_back(
                    vk::RenderingAttachmentInfo{}
                        .setImageView(colorImage->getImageView())
                        .setImageLayout(colorImage->getLayout())
                        .setLoadOp(colorAttachment->loadOp)
                        .setStoreOp(colorAttachment->storeOp)
                        .setClearValue(colorAttachment->clearValue)
                );
            }
        }

        // Rendering info

        vk::RenderingInfo renderingInfo{};
        renderingInfo
            .setRenderArea({{0, 0}, extent})
            .setLayerCount(1)
            .setColorAttachments(colorAttachmentsInfo);

        // Attach depth buffer

        const VulkanRenderPassAttachment* depthAttachment = pass->getDepthAttachment();

        VulkanImage* depthImage = _resources->getDepthBufferAttachment()->resource.image;

        vk::ImageLayout targetDepthLayout;

        if (depthAttachment) {
            // Pass writes depth
            targetDepthLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        } else {
            // Pass reads depth
            targetDepthLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        }

        // Depth image transition
        TRY(depthImage->transitionLayout(commandBuffer, errorMessage, targetDepthLayout));

        if (depthAttachment) {
            vk::RenderingAttachmentInfo depthAttachmentInfo{};
            depthAttachmentInfo
                .setImageView(depthImage->getImageView())
                .setImageLayout(depthImage->getLayout())
                .setLoadOp(depthAttachment->loadOp)
                .setStoreOp(depthAttachment->storeOp)
                .setClearValue(depthAttachment->clearValue);

            renderingInfo.setPDepthAttachment(&depthAttachmentInfo);
        }

        commandBuffer.beginRendering(renderingInfo);

        if (!isFullScreenPass) {
            commandBuffer.beginQuery(_queryPool, 0, {});
        }

        commandBuffer.bindPipeline(pass->getBindPoint(), *pass->getPipeline());

        for (const auto& drawCall : pass->getDrawCalls()) {
            const auto& draw = *drawCall;

            const vk::PipelineLayout pipelineLayout = pass->getPipeline()->getLayout();

            /*---------------------------------------*/
            /*            Bind Resources             */
            /*---------------------------------------*/

            // Descriptors

            std::vector<vk::DescriptorSet> descriptorSets{};
            descriptorSets.push_back(_frame->getDescriptors()->getSet(frameIndex));

            for (const auto& drawDescriptorSets : draw.descriptorSets) {
                descriptorSets.push_back(drawDescriptorSets->getSet(frameIndex));
            }

            for (const auto& passDescriptorSets : pass->getDescriptorSets() | std::views::values) {
                descriptorSets.push_back(passDescriptorSets->getSet(frameIndex));
            }

            if (!descriptorSets.empty()) {
                commandBuffer.bindDescriptorSets(
                    pass->getBindPoint(), pipelineLayout, 0, descriptorSets, nullptr
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

        if (!isFullScreenPass) {
            commandBuffer.endQuery(_queryPool, 0);
        }

        commandBuffer.endRendering();

        for (const auto& [resource, targetLayout] : pass->getTransitions()) {
            VulkanImage* resourceImage = resource->resolveImage();

            TRY(resourceImage->transitionLayout(commandBuffer, errorMessage, targetLayout));
        }
    }

    return true;
}
