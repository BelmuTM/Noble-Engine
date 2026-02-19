#include "VulkanDrawCall.h"

void VulkanDrawCall::record(
    const vk::CommandBuffer    commandBuffer,
    const vk::Extent2D         extent,
    const vk::PipelineLayout   pipelineLayout,
    const VulkanShaderProgram* shaderProgram
) const {
    if (!_mesh) return;

    commandBuffer.setViewport(0, resolveViewport(extent));
    commandBuffer.setScissor(0, resolveScissor(extent));

    pushConstants(commandBuffer, pipelineLayout, shaderProgram);

    if (!_mesh->isBufferless() && _mesh->getVertexBuffer()) {
        const vk::Buffer&    vertexBuffer = _mesh->getVertexBuffer()->handle();
        const vk::DeviceSize vertexOffset = _mesh->getVertexOffset();

        commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer, &vertexOffset);

        if (_mesh->getIndexBuffer()) {
            const vk::Buffer& indexBuffer    = _mesh->getIndexBuffer()->handle();
            const vk::DeviceSize indexOffset = _mesh->getIndexOffset();

            commandBuffer.bindIndexBuffer(indexBuffer, indexOffset, vk::IndexType::eUint32);

            commandBuffer.drawIndexed(_mesh->getIndices().size(), 1, 0, 0, 0);
        }
    } else {
        commandBuffer.draw(_mesh->getVertices().size(), 1, 0, 0);
    }
}

void VulkanDrawCall::pushConstants(
    const vk::CommandBuffer    commandBuffer,
    const vk::PipelineLayout   pipelineLayout,
    const VulkanShaderProgram* shaderProgram
) const noexcept {
    if (_pushConstants.empty()) return;

    for (const auto& [name, range] : shaderProgram->getPushConstants()) {
        if (auto it = _pushConstants.find(name); it != _pushConstants.end()) {
            it->second->push(commandBuffer, pipelineLayout, range);
        }
    }
}

vk::Viewport VulkanDrawCall::resolveViewport(const vk::Extent2D extent) const {
    if (_viewport) return *_viewport;
    return vk::Viewport{
        0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f
    };
}

vk::Rect2D VulkanDrawCall::resolveScissor(const vk::Extent2D extent) const {
    if (_scissor) return *_scissor;
    return {vk::Offset2D(0, 0), extent};
}
