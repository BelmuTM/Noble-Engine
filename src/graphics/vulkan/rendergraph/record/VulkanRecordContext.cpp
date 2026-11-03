#include "VulkanRecordContext.h"

#include "graphics/vulkan/pipeline/VulkanPipeline.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorSets.h"

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
