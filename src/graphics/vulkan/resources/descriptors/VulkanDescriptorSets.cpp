#include "VulkanDescriptorSets.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

Expected<void> VulkanDescriptorSets::allocate() {
    if (!_manager) {
        return VK_FAIL("Failed to allocate descriptor sets: descriptor manager is null.");
    }

    std::vector layouts(_manager->getFramesInFlight(), _manager->getLayout());

    vk::DescriptorSetAllocateInfo descriptorSetInfo{};
    descriptorSetInfo
        .setDescriptorPool(_manager->getPool())
        .setSetLayouts(layouts);

    _descriptorSets.clear();
    _descriptorSets.reserve(_manager->getFramesInFlight());

    return _manager->allocateSets(_descriptorSets, descriptorSetInfo);
}

void VulkanDescriptorSets::updateDescriptorSets(const VulkanDescriptorInfo& info, const std::uint32_t frameIndex) const {
    if (!_manager) return;

    vk::WriteDescriptorSet descriptorSetWrite{};
    descriptorSetWrite
        .setDstSet(_descriptorSets[frameIndex])
        .setDstBinding(info.binding)
        .setDstArrayElement(0)
        .setDescriptorCount(1)
        .setDescriptorType(info.type);

    if (info.type == vk::DescriptorType::eUniformBuffer || info.type == vk::DescriptorType::eStorageBuffer)
        descriptorSetWrite.setBufferInfo(info.bufferInfo);
    else if (info.type == vk::DescriptorType::eCombinedImageSampler)
        descriptorSetWrite.setImageInfo(info.imageInfo);

    _manager->updateSets(descriptorSetWrite);
}

void VulkanDescriptorSets::updatePerFrameDescriptorSets(const VulkanDescriptorInfo& info) const {
    for (std::uint32_t i = 0; i < _manager->getFramesInFlight(); i++) {
        updateDescriptorSets(info, i);
    }
}

void VulkanDescriptorSets::updatePerFrameUBODescriptorSets(
    const VulkanUniformBufferBase& ubo, const std::uint32_t binding
) const {
    for (std::uint32_t i = 0; i < _manager->getFramesInFlight(); i++) {
        updateDescriptorSets(ubo.getDescriptorInfo(binding, i), i);
    }
}

void VulkanDescriptorSets::updatePerFrameSSBODescriptorSets(
    const VulkanStorageBuffer& ssbo, const std::uint32_t binding
) const {
    for (std::uint32_t i = 0; i < _manager->getFramesInFlight(); i++) {
        updateDescriptorSets(ssbo.getDescriptorInfo(binding, i), i);
    }
}
