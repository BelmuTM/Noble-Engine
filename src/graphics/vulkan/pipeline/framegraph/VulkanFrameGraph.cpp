#include "VulkanFrameGraph.h"

bool VulkanFrameGraph::create(
    const VulkanMeshManager& meshManager, const vk::QueryPool queryPool, std::string& errorMessage
) noexcept {
    _meshManager = &meshManager;
    _queryPool   = queryPool;
    return true;
}

void VulkanFrameGraph::destroy() noexcept {
    _meshManager = nullptr;
}

void VulkanFrameGraph::attachSwapchainOutput() const {
    VulkanFramePass* lastPass = _passes.back().get();

    VulkanFramePassResource swapchainOutput{};
    swapchainOutput
        .setType(SwapchainOutput)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setResolveImageView([](const VulkanFrameContext& frame) { return frame.swapchainImageView; });

    VulkanFramePassAttachment swapchainAttachment{};
    swapchainAttachment
        .setResource(swapchainOutput)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(defaultClearColor);

    lastPass->addColorAttachment(swapchainAttachment);
}

void VulkanFrameGraph::execute(const VulkanFrameContext& frame) const {
    for (const auto& pass : _passes) {
        executePass(pass.get(), frame);
    }
}

void VulkanFrameGraph::executePass(const VulkanFramePass* pass, const VulkanFrameContext& frame) const {
    const vk::Buffer& vertexBuffer = _meshManager->getVertexBuffer();
    const vk::Buffer& indexBuffer  = _meshManager->getIndexBuffer();

    if (pass->getBindPoint() == vk::PipelineBindPoint::eGraphics) {
        std::vector<vk::RenderingAttachmentInfo> colorAttachmentsInfo{};

        for (const auto& [resource, loadOp, storeOp, clearValue] : pass->getColorAttachments()) {
            colorAttachmentsInfo.push_back(vk::RenderingAttachmentInfo{}
                .setImageView(resource.resolveImageView(frame))
                .setImageLayout(resource.layout)
                .setLoadOp(loadOp)
                .setStoreOp(storeOp)
                .setClearValue(clearValue)
            );
        }

        const VulkanFramePassAttachment& depthAttachment = pass->getDepthAttachment();
        vk::RenderingAttachmentInfo depthAttachmentInfo{};

        if (depthAttachment) {
            depthAttachmentInfo
                .setImageView(depthAttachment.resource.resolveImageView(frame))
                .setImageLayout(depthAttachment.resource.layout)
                .setLoadOp(depthAttachment.loadOp)
                .setStoreOp(depthAttachment.storeOp)
                .setClearValue(depthAttachment.clearValue);
        }

        vk::RenderingInfo renderingInfo{};
        renderingInfo
            .setRenderArea({{0, 0}, frame.extent})
            .setLayerCount(1)
            .setColorAttachments(colorAttachmentsInfo)
            .setPDepthAttachment(&depthAttachmentInfo);

        frame.cmdBuffer.resetQueryPool(_queryPool, 0, 1);

        frame.cmdBuffer.beginRendering(renderingInfo);

        frame.cmdBuffer.beginQuery(_queryPool, 0, {});

        frame.cmdBuffer.bindPipeline(pass->getBindPoint(), *pass->getPipeline());

        for (const auto& drawCall : pass->getDrawCalls()) {
            const auto& draw = *drawCall;

            const vk::PipelineLayout pipelineLayout = pass->getPipeline()->getLayout();

            std::vector<vk::DescriptorSet> descriptorSets{};
            descriptorSets.push_back(frame.frameDescriptors.at(frame.frameIndex));

            if (draw.descriptorResolver) {
                if (const auto objectSet = draw.descriptorResolver(frame); !objectSet.empty()) {
                    descriptorSets.push_back(objectSet[0]);
                }
            }

            if (!descriptorSets.empty()) {
                frame.cmdBuffer.bindDescriptorSets(
                    pass->getBindPoint(), pipelineLayout, 0, descriptorSets, nullptr
                );
            }

            if (auto* drawPushConstant = dynamic_cast<const DrawCallPushConstantBase*>(&draw)) {
                drawPushConstant->pushConstants(
                    frame.cmdBuffer, pipelineLayout, pass->getPipeline()->getStageFlags(), frame
                );
            }

            frame.cmdBuffer.setViewport(0, draw.resolveViewport(frame));
            frame.cmdBuffer.setScissor(0, draw.resolveScissor(frame));

            const vk::DeviceSize vertexOffset = draw.mesh.getVertexOffset();
            const vk::DeviceSize indexOffset  = draw.mesh.getIndexOffset();

            if (!draw.mesh.isBufferless()) {
                frame.cmdBuffer.bindVertexBuffers(0, 1, &vertexBuffer, &vertexOffset);
                frame.cmdBuffer.bindIndexBuffer(indexBuffer, indexOffset, vk::IndexType::eUint32);
                frame.cmdBuffer.drawIndexed(draw.mesh.getIndices().size(), 1, 0, 0, 0);
            } else {
                frame.cmdBuffer.draw(draw.mesh.getVertices().size(), 1, 0, 0);
            }
        }

        frame.cmdBuffer.endQuery(_queryPool, 0);

        frame.cmdBuffer.endRendering();
    }
}
