#include "VulkanRecordContext.h"

#include "core/render/BindingSlots.h"

#include "graphics/vulkan/rendergraph/draw/VulkanFrameCuller.h"
#include "graphics/vulkan/resources/descriptors/VulkanDescriptorSets.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

void VulkanRecordContext::bindGlobalDescriptors(
    const vk::PipelineLayout pipelineLayout, const vk::PipelineBindPoint pipelineBindPoint
) const {
    const std::array fixedSets = {
        _frame->getDescriptorSets()->getSet(_frameIndex),                // slot 0: FrameData
        _renderObjectManager->getDescriptorSets()->getSet(_frameIndex),  // slot 1: ObjectData
        _frameCuller->getDescriptorSets()->getSet(_frameIndex),          // slot 2: CullingData
    };

    _commandBuffer.bindDescriptorSets(pipelineBindPoint, pipelineLayout, 0, fixedSets, nullptr);
}

void VulkanRecordContext::bindPassDescriptors(
    const vk::PipelineLayout    pipelineLayout,
    const vk::PipelineBindPoint pipelineBindPoint,
    const VulkanDescriptorSets* sets
) const {
    // slot 3: PassData
    if (sets) {
        _commandBuffer.bindDescriptorSets(
            pipelineBindPoint, pipelineLayout,
            BindingSlots::PassData,
            sets->getSet(_frameIndex),
            nullptr
        );
    }
}

void VulkanRecordContext::bindMaterial(
    const vk::PipelineLayout    pipelineLayout,
    const vk::PipelineBindPoint pipelineBindPoint,
    const VulkanMaterial*       material
) const {
    if (material) {
        // slot 4: MaterialData
        const vk::DescriptorSet materialSet = material->getDescriptorSets()->getSet(_frameIndex);

        _commandBuffer.bindDescriptorSets(
            pipelineBindPoint, pipelineLayout, BindingSlots::MaterialData, materialSet, nullptr
        );
    }
}
