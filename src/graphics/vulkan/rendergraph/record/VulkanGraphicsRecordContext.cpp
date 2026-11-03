#include "VulkanGraphicsRecordContext.h"

#include "graphics/vulkan/pipeline/VulkanPipeline.h"

#include "graphics/vulkan/resources/materials/VulkanMaterial.h"

void VulkanGraphicsRecordContext::bindMaterial(const VulkanMaterial* material) {
    if (!material || material == _lastBoundMaterial) return;

    _commandBuffer.bindDescriptorSets(
        _pass.pipeline->getBindPoint(), _pass.pipeline->getLayout(),
        BindingSlots::MaterialData,
        material->getDescriptorSets()->getSet(_frame.frameIndex), nullptr
    );

    _lastBoundMaterial = material;
}

void VulkanGraphicsRecordContext::bindGeometryBuffers() const {
    _commandBuffer.bindVertexBuffers(0, _frame.vertexBuffer, vk::DeviceSize{0});
    _commandBuffer.bindIndexBuffer(_frame.indexBuffer, 0, vk::IndexType::eUint32);
}

void VulkanGraphicsRecordContext::draw(
    const std::uint32_t vertexCount,
    const std::uint32_t instanceCount,
    const std::uint32_t firstVertex,
    const std::uint32_t firstInstance
) const {
    _commandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanGraphicsRecordContext::drawBatch(const VulkanDrawCommand& batch) {
    bindMaterial(batch.material);

    const auto& cmd = batch.command;
    _commandBuffer.drawIndexed(cmd.indexCount, cmd.instanceCount, cmd.firstIndex, cmd.vertexOffset, cmd.firstInstance);
}

void VulkanGraphicsRecordContext::drawBatches() {
    if (_drawBatches.empty()) return;

    bindGeometryBuffers();

    for (const VulkanDrawCommand& batch : _drawBatches) {
        drawBatch(batch);
    }
}

void VulkanGraphicsRecordContext::setViewport() const {
    const vk::Viewport viewport{
        0.0f, 0.0f, static_cast<float>(_frame.extent.width), static_cast<float>(_frame.extent.height), 0.0f, 1.0f
    };

    _commandBuffer.setViewport(0, viewport);
}

void VulkanGraphicsRecordContext::setScissor() const {
    const vk::Rect2D scissor{vk::Offset2D(0, 0), _frame.extent};

    _commandBuffer.setScissor(0, scissor);
}
