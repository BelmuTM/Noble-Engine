#include "VulkanFrameGraph.h"

bool VulkanFrameGraph::create(const VulkanMeshManager& meshManager, std::string& errorMessage) noexcept {
    _meshManager = &meshManager;
    return true;
}

void VulkanFrameGraph::destroy() noexcept {
    _meshManager = nullptr;
}

void VulkanFrameGraph::executePass(const FramePass& pass, const vk::CommandBuffer commandBuffer) const {
    const vk::Buffer& vertexBuffer = _meshManager->getVertexBuffer();
    const vk::Buffer& indexBuffer  = _meshManager->getIndexBuffer();

    if (pass.bindPoint == vk::PipelineBindPoint::eGraphics) {
        commandBuffer.beginRendering(pass.renderingInfo);

        for (auto& draw : pass.drawCalls) {
            commandBuffer.bindPipeline(pass.bindPoint, *draw.pipeline);

            const vk::DeviceSize vertexOffset = draw.mesh.getVertexOffset();
            const vk::DeviceSize indexOffset  = draw.mesh.getIndexOffset();

            commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer, &vertexOffset);
            commandBuffer.bindIndexBuffer(indexBuffer, indexOffset, vk::IndexType::eUint16);

            commandBuffer.bindDescriptorSets(
                pass.bindPoint, draw.pipeline->getLayout(), 0, *draw.descriptorSets, nullptr
            );

            if (draw.viewport) commandBuffer.setViewport(0, *draw.viewport);
            if (draw.scissor)  commandBuffer.setScissor(0, *draw.scissor);

            commandBuffer.drawIndexed(draw.mesh.getIndices().size(), 1, 0, 0, 0);
        }

        commandBuffer.endRendering();
    }
}
