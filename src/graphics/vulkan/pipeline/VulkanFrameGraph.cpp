#include "VulkanFrameGraph.h"

#include <iostream>

bool VulkanFrameGraph::create(const VulkanMeshManager& meshManager, std::string& errorMessage) noexcept {
    _meshManager = &meshManager;
    return true;
}

void VulkanFrameGraph::destroy() noexcept {
    _meshManager = nullptr;
}

void VulkanFrameGraph::execute(const FrameContext& frame) const {
    for (auto& pass : _passes) {
        executePass(pass, frame);
    }
}

void VulkanFrameGraph::executePass(const FramePass& pass, const FrameContext& frame) const {
    const vk::Buffer& vertexBuffer = _meshManager->getVertexBuffer();
    const vk::Buffer& indexBuffer  = _meshManager->getIndexBuffer();

    if (pass.bindPoint == vk::PipelineBindPoint::eGraphics) {
        std::vector<vk::RenderingAttachmentInfo> colorAttachmentsInfo{};

        for (const auto& [resource, loadOp, storeOp, clearValue] : pass.colorAttachments) {
            colorAttachmentsInfo.push_back(vk::RenderingAttachmentInfo{}
                .setImageView(resource.resolveImageView(frame))
                .setImageLayout(resource.layout)
                .setLoadOp(loadOp)
                .setStoreOp(storeOp)
                .setClearValue(clearValue)
            );
        }

        vk::RenderingAttachmentInfo depthAttachmentInfo{};
        depthAttachmentInfo
            .setImageView(pass.depthAttachment.resource.resolveImageView(frame))
            .setImageLayout(pass.depthAttachment.resource.layout)
            .setLoadOp(pass.depthAttachment.loadOp)
            .setStoreOp(pass.depthAttachment.storeOp)
            .setClearValue(pass.depthAttachment.clearValue);

        vk::RenderingInfo renderingInfo{};
        renderingInfo
            .setRenderArea({{0, 0}, frame.extent})
            .setLayerCount(1)
            .setColorAttachments(colorAttachmentsInfo)
            .setPDepthAttachment(&depthAttachmentInfo);

        frame.cmdBuffer.beginRendering(renderingInfo);

        for (auto& draw : pass.drawCalls) {
            frame.cmdBuffer.bindPipeline(pass.bindPoint, *draw.pipeline);

            const vk::DeviceSize vertexOffset = draw.mesh.getVertexOffset();
            const vk::DeviceSize indexOffset  = draw.mesh.getIndexOffset();

            frame.cmdBuffer.bindVertexBuffers(0, 1, &vertexBuffer, &vertexOffset);
            frame.cmdBuffer.bindIndexBuffer(indexBuffer, indexOffset, vk::IndexType::eUint16);

            frame.cmdBuffer.bindDescriptorSets(
                pass.bindPoint, draw.pipeline->getLayout(), 0, draw.descriptorResolver(frame), nullptr
            );

            frame.cmdBuffer.setViewport(0, draw.resolveViewport(frame));
            frame.cmdBuffer.setScissor(0, draw.resolveScissor(frame));

            frame.cmdBuffer.drawIndexed(draw.mesh.getIndices().size(), 1, 0, 0, 0);
        }

        frame.cmdBuffer.endRendering();
    }
}
