#include "VulkanDrawCall.h"

#include <ranges>

void VulkanDrawCall::record(
    const vk::CommandBuffer  commandBuffer,
    const vk::Extent2D       extent,
    const vk::PipelineLayout pipelineLayout,
    const std::uint32_t      instanceCount,
    const std::uint32_t      firstInstance
) const {
    if (!_renderMesh.mesh) return;

    commandBuffer.setViewport(0, resolveViewport(extent));
    commandBuffer.setScissor(0, resolveScissor(extent));

    pushConstants(commandBuffer, pipelineLayout);

    const VulkanMesh& mesh = *_renderMesh.mesh;

    if (!mesh.isBufferless() && mesh.getVertexBuffer()) {
        const vk::Buffer&    vertexBuffer = mesh.getVertexBuffer()->handle();
        const vk::DeviceSize vertexOffset = mesh.getVertexOffset();

        commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer, &vertexOffset);

        if (mesh.getIndexBuffer()) {
            const vk::Buffer& indexBuffer    = mesh.getIndexBuffer()->handle();
            const vk::DeviceSize indexOffset = mesh.getIndexOffset();

            commandBuffer.bindIndexBuffer(indexBuffer, indexOffset, vk::IndexType::eUint32);

            commandBuffer.drawIndexed(
                static_cast<std::uint32_t>(mesh.getIndices().size()), instanceCount, 0, 0, firstInstance
            );
        }
    } else {
        commandBuffer.draw(static_cast<std::uint32_t> (mesh.getVertices().size()), 1, 0, 0);
    }
}

void VulkanDrawCall::pushConstants(
    const vk::CommandBuffer commandBuffer, const vk::PipelineLayout pipelineLayout
) const noexcept {
    if (_pushConstants.empty()) return;

    for (const auto& [data, range] : _pushConstants | std::views::values) {
        commandBuffer.pushConstants(pipelineLayout, range.stageFlags, range.offset, range.size, data);
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
