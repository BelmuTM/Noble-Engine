#include "VulkanRecordContext.h"

#include "graphics/vulkan/pipeline/VulkanPipeline.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorSets.h"
#include "graphics/vulkan/resources/materials/VulkanMaterial.h"

void VulkanRecordContext::bindPipeline() const {
    _commandBuffer.bindPipeline(_pass.pipeline->getBindPoint(), _pass.pipeline->handle());
}

void VulkanRecordContext::bindGlobalDescriptors() const {
    _commandBuffer.bindDescriptorSets(
        _pass.pipeline->getBindPoint(), _pass.pipeline->getLayout(),
        BindingSlots::FrameData,
        _frame.globalSets, nullptr
    );
}

void VulkanRecordContext::bindPassDescriptors() const {
    if (!_pass.sets) return;

    _commandBuffer.bindDescriptorSets(
        _pass.pipeline->getBindPoint(), _pass.pipeline->getLayout(),
        BindingSlots::PassData,
        _pass.sets->getSet(_frame.frameIndex), nullptr
    );
}

void VulkanRecordContext::bindMaterial(const VulkanMaterial* material) {
    if (!material || material == _lastBoundMaterial) return;

    _commandBuffer.bindDescriptorSets(
        _pass.pipeline->getBindPoint(), _pass.pipeline->getLayout(),
        BindingSlots::MaterialData,
        material->getDescriptorSets()->getSet(_frame.frameIndex), nullptr
    );

    _lastBoundMaterial = material;
}

void VulkanRecordContext::bindGeometryBuffers() const {
    _commandBuffer.bindVertexBuffers(0, _frame.vertexBuffer, vk::DeviceSize{0});
    _commandBuffer.bindIndexBuffer(_frame.indexBuffer, 0, vk::IndexType::eUint32);
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
    const auto& ranges = _pass.layoutDescriptor->pushConstantRanges;

    const auto rangeIt = ranges.find(name);
    assert(rangeIt != ranges.end());

    const auto& [stageFlags, offset, size] = rangeIt->second;
    assert(size == typeSize);

    _commandBuffer.pushConstants(_pass.pipeline->getLayout(), stageFlags, offset, size, data);
}
