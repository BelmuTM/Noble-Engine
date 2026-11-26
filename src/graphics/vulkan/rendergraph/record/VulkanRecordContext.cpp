#include "VulkanRecordContext.h"

#include "graphics/vulkan/pipeline/VulkanPipeline.h"
#include "graphics/vulkan/rendergraph/draw/VulkanDrawBatchBuilder.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorSets.h"
#include "graphics/vulkan/resources/materials/VulkanMaterial.h"

void VulkanRecordContext::bindPipeline(const std::size_t slot) {
    _boundPipeline = _pass.pipelines[slot];
    _commandBuffer.bindPipeline(_boundPipeline->getBindPoint(), _boundPipeline->handle());
}

void VulkanRecordContext::bindGlobalDescriptors() const {
    _commandBuffer.bindDescriptorSets(
        _boundPipeline->getBindPoint(), _boundPipeline->getLayout(),
        BindingSlots::FrameData,
        _frame.globalSets, nullptr
    );
}

void VulkanRecordContext::bindPassDescriptors() const {
    if (!_pass.sets) return;

    _commandBuffer.bindDescriptorSets(
        _boundPipeline->getBindPoint(), _boundPipeline->getLayout(),
        BindingSlots::PassData,
        _pass.sets->getSet(_frame.frameIndex), nullptr
    );
}

void VulkanRecordContext::bindMaterial(const VulkanMaterial* material) {
    if (!material || material == _boundMaterial) return;

    _commandBuffer.bindDescriptorSets(
        _boundPipeline->getBindPoint(), _boundPipeline->getLayout(),
        BindingSlots::MaterialData,
        material->getDescriptorSets()->getSet(_frame.frameIndex), nullptr
    );

    _boundMaterial = material;
}

void VulkanRecordContext::bindGeometryBuffers() const {
    _commandBuffer.bindVertexBuffers(0, _frame.vertexBuffer, vk::DeviceSize{0});
    _commandBuffer.bindIndexBuffer(_frame.indexBuffer, 0, vk::IndexType::eUint32);
}

void VulkanRecordContext::draw(
    const std::uint32_t vertexCount,
    const std::uint32_t instanceCount,
    const std::uint32_t firstVertex,
    const std::uint32_t firstInstance
) const {
    _commandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanRecordContext::drawBatch(const VulkanDrawCommand& batch) {
    bindMaterial(batch.material);

    const auto& cmd = batch.command;
    _commandBuffer.drawIndexed(cmd.indexCount, cmd.instanceCount, cmd.firstIndex, cmd.vertexOffset, cmd.firstInstance);
}

void VulkanRecordContext::drawBatches(const DrawListId drawListId) {
    const auto& drawBatches = _frame.drawBatchBuilder->getDrawList(drawListId);

    if (drawBatches.empty()) return;

    bindGeometryBuffers();

    for (const VulkanDrawCommand& batch : drawBatches) {
        drawBatch(batch);
    }
}

void VulkanRecordContext::setViewport() const {
    const vk::Viewport viewport{
        0.0f, 0.0f, static_cast<float>(_frame.extent.width), static_cast<float>(_frame.extent.height), 0.0f, 1.0f
    };

    _commandBuffer.setViewport(0, viewport);
}

void VulkanRecordContext::setScissor() const {
    const vk::Rect2D scissor{vk::Offset2D(0, 0), _frame.extent};

    _commandBuffer.setScissor(0, scissor);
}

void VulkanRecordContext::pushConstantRaw(
    const std::string& name, const void* data, const std::uint32_t typeSize
) const {
    const auto& ranges = _boundPipeline->getPushConstantRanges();

    const auto rangeIt = ranges.find(name);
    assert(rangeIt != ranges.end());

    const auto& [stageFlags, offset, size] = rangeIt->second;
    assert(size == typeSize);

    _commandBuffer.pushConstants(_boundPipeline->getLayout(), stageFlags, offset, size, data);
}

void VulkanRecordContext::beginQuery() const {
    _commandBuffer.beginQuery(_frame.queryPool, 0, {});
}

void VulkanRecordContext::endQuery() const {
    _commandBuffer.endQuery(_frame.queryPool, 0);
}
