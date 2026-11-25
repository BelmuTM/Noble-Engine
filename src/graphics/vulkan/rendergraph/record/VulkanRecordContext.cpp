#include "VulkanRecordContext.h"

#include "graphics/vulkan/pipeline/VulkanPipeline.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorSets.h"

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
