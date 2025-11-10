#include "VulkanFrameGraph.h"

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

        if (pass.depthAttachment) {
            depthAttachmentInfo
                .setImageView(pass.depthAttachment.resource.resolveImageView(frame))
                .setImageLayout(pass.depthAttachment.resource.layout)
                .setLoadOp(pass.depthAttachment.loadOp)
                .setStoreOp(pass.depthAttachment.storeOp)
                .setClearValue(pass.depthAttachment.clearValue);
        }

        vk::RenderingInfo renderingInfo{};
        renderingInfo
            .setRenderArea({{0, 0}, frame.extent})
            .setLayerCount(1)
            .setColorAttachments(colorAttachmentsInfo)
            .setPDepthAttachment(&depthAttachmentInfo);

        frame.cmdBuffer.beginRendering(renderingInfo);

        frame.cmdBuffer.bindPipeline(pass.bindPoint, *pass.pipeline);

        for (const auto& drawCall : pass.drawCalls) {
            const auto& draw = *drawCall;

            const vk::PipelineLayout pipelineLayout = pass.pipeline->getLayout();

            std::vector<vk::DescriptorSet> descriptorSets{};
            descriptorSets.push_back(frame.frameDescriptors.at(frame.frameIndex));

            if (draw.descriptorResolver) {
                if (const auto objectSet = draw.descriptorResolver(frame); !objectSet.empty()) {
                    descriptorSets.push_back(objectSet[0]);
                }
            }

            if (!descriptorSets.empty()) {
                frame.cmdBuffer.bindDescriptorSets(
                    pass.bindPoint, pipelineLayout, 0, descriptorSets, nullptr
                );
            }

            if (auto* drawPushConstant = dynamic_cast<const DrawCallPushConstantBase*>(&draw)) {
                drawPushConstant->pushConstants(
                    frame.cmdBuffer, pipelineLayout, pass.pipeline->getStageFlags(), frame
                );
            }

            frame.cmdBuffer.setViewport(0, draw.resolveViewport(frame));
            frame.cmdBuffer.setScissor(0, draw.resolveScissor(frame));

            const vk::DeviceSize vertexOffset = draw.mesh.getVertexOffset();
            const vk::DeviceSize indexOffset  = draw.mesh.getIndexOffset();

            if (!draw.mesh.isBufferless()) {
                frame.cmdBuffer.bindVertexBuffers(0, 1, &vertexBuffer, &vertexOffset);
                frame.cmdBuffer.bindIndexBuffer(indexBuffer, indexOffset, vk::IndexType::eUint16);
                frame.cmdBuffer.drawIndexed(draw.mesh.getIndices().size(), 1, 0, 0, 0);
            } else {
                frame.cmdBuffer.draw(draw.mesh.getVertices().size(), 1, 0, 0);
            }
        }

        frame.cmdBuffer.endRendering();
    }
}
