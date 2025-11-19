#include "VulkanRenderGraph.h"

bool VulkanRenderGraph::create(
    const VulkanMeshManager&    meshManager,
    const VulkanFrameResources& frame,
    const vk::QueryPool         queryPool,
    std::string&                errorMessage
) noexcept {
    _meshManager = &meshManager;
    _frame       = &frame;
    _queryPool   = queryPool;
    return true;
}

void VulkanRenderGraph::destroy() noexcept {
    _meshManager = nullptr;
    _frame       = nullptr;
}

void VulkanRenderGraph::attachSwapchainOutput(const VulkanSwapchain& swapchain) const {
    VulkanRenderPassResource swapchainOutput{};
    swapchainOutput
        .setType(SwapchainOutput)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setFormat(swapchain.getFormat())
        .setResolveImageView([](const VulkanFrameContext& frame) { return frame.swapchainImageView; });

    VulkanRenderPassAttachment swapchainAttachment{};
    swapchainAttachment
        .setResource(swapchainOutput)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(defaultClearColor);

    VulkanRenderPass* lastPass = _passes.back().get();
    lastPass->addColorAttachmentAtIndex(0, swapchainAttachment);
}

void VulkanRenderGraph::execute(const vk::CommandBuffer commandBuffer) const {
    for (const auto& pass : _passes) {
        executePass(commandBuffer, pass.get());
    }
}

void VulkanRenderGraph::executePass(const vk::CommandBuffer commandBuffer, const VulkanRenderPass* pass) const {
    const vk::Buffer& vertexBuffer = _meshManager->getVertexBuffer();
    const vk::Buffer& indexBuffer  = _meshManager->getIndexBuffer();

    const VulkanFrameContext&  frameContext  = _frame->getFrameContext();
    const VulkanShaderProgram* shaderProgram = pass->getPipelineDescriptor().shaderProgram;

    if (pass->getBindPoint() == vk::PipelineBindPoint::eGraphics) {
        /*---------------------------------------*/
        /*     Rendering Info & Attachments      */
        /*---------------------------------------*/

        std::vector<vk::RenderingAttachmentInfo> colorAttachmentsInfo{};

        for (const auto& [resource, loadOp, storeOp, clearValue] : pass->getColorAttachments()) {
            colorAttachmentsInfo.push_back(vk::RenderingAttachmentInfo{}
                .setImageView(resource.resolveImageView(_frame->getFrameContext()))
                .setImageLayout(resource.layout)
                .setLoadOp(loadOp)
                .setStoreOp(storeOp)
                .setClearValue(clearValue)
            );
        }

        const VulkanRenderPassAttachment& depthAttachment = pass->getDepthAttachment();
        vk::RenderingAttachmentInfo depthAttachmentInfo{};

        if (depthAttachment) {
            depthAttachmentInfo
                .setImageView(depthAttachment.resource.resolveImageView(frameContext))
                .setImageLayout(depthAttachment.resource.layout)
                .setLoadOp(depthAttachment.loadOp)
                .setStoreOp(depthAttachment.storeOp)
                .setClearValue(depthAttachment.clearValue);
        }

        vk::RenderingInfo renderingInfo{};
        renderingInfo
            .setRenderArea({{0, 0}, frameContext.extent})
            .setLayerCount(1)
            .setColorAttachments(colorAttachmentsInfo)
            .setPDepthAttachment(&depthAttachmentInfo);

        commandBuffer.resetQueryPool(_queryPool, 0, 1);

        commandBuffer.beginRendering(renderingInfo);

        commandBuffer.beginQuery(_queryPool, 0, {});

        commandBuffer.bindPipeline(pass->getBindPoint(), *pass->getPipeline());

        for (const auto& drawCall : pass->getDrawCalls()) {
            const auto& draw = *drawCall;

            const vk::PipelineLayout pipelineLayout = pass->getPipeline()->getLayout();

            /*---------------------------------------*/
            /*            Bind Resources             */
            /*---------------------------------------*/

            std::vector<vk::DescriptorSet> descriptorSets{};
            descriptorSets.push_back(_frame->getDescriptors()->getSets().at(frameContext.frameIndex));

            if (draw.descriptorResolver) {
                if (const auto objectSet = draw.descriptorResolver(frameContext); !objectSet.empty()) {
                    descriptorSets.push_back(objectSet[0]);
                }
            }

            if (!descriptorSets.empty()) {
                commandBuffer.bindDescriptorSets(
                    pass->getBindPoint(), pipelineLayout, 0, descriptorSets, nullptr
                );
            }

            if (auto* drawPushConstant = dynamic_cast<const VulkanDrawCallWithPushConstants*>(&draw)) {
                drawPushConstant->pushConstants(commandBuffer, pipelineLayout, shaderProgram);
            }

            /*---------------------------------------*/
            /*              Draw Meshes              */
            /*---------------------------------------*/

            commandBuffer.setViewport(0, draw.resolveViewport(frameContext));
            commandBuffer.setScissor(0, draw.resolveScissor(frameContext));

            const vk::DeviceSize vertexOffset = draw.mesh->getVertexOffset();
            const vk::DeviceSize indexOffset  = draw.mesh->getIndexOffset();

            if (!draw.mesh->isBufferless()) {
                commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer, &vertexOffset);
                commandBuffer.bindIndexBuffer(indexBuffer, indexOffset, vk::IndexType::eUint32);
                commandBuffer.drawIndexed(draw.mesh->getIndices().size(), 1, 0, 0, 0);
            } else {
                commandBuffer.draw(draw.mesh->getVertices().size(), 1, 0, 0);
            }
        }

        commandBuffer.endQuery(_queryPool, 0);

        commandBuffer.endRendering();
    }
}
