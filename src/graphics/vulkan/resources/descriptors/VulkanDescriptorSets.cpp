#include "VulkanDescriptorSets.h"

bool VulkanDescriptorSets::allocate(std::string& errorMessage) {
    if (!_manager) {
        errorMessage = "Failed to allocate Vulkan descriptor sets: descriptor manager is null";
        return false;
    }

    std::vector layouts(_manager->getFramesInFlight(), _manager->getLayout());

    vk::DescriptorSetAllocateInfo descriptorSetInfo{};
    descriptorSetInfo
        .setDescriptorPool(_manager->getPool())
        .setSetLayouts(layouts);

    _descriptorSets.clear();
    _descriptorSets.reserve(_manager->getFramesInFlight());

    return _manager->allocateSets(_descriptorSets, descriptorSetInfo, errorMessage);
}

void VulkanDescriptorSets::bindResource(const VulkanDescriptorInfo& info, const uint32_t frameIndex) const {
    if (!_manager) return;

    vk::WriteDescriptorSet descriptorSetWrite{};
    descriptorSetWrite
        .setDstSet(_descriptorSets[frameIndex])
        .setDstBinding(info.binding)
        .setDstArrayElement(0)
        .setDescriptorCount(1)
        .setDescriptorType(info.type);

    if (info.type == vk::DescriptorType::eUniformBuffer)
        descriptorSetWrite.setBufferInfo(info.bufferInfo);
    else if (info.type == vk::DescriptorType::eCombinedImageSampler)
        descriptorSetWrite.setImageInfo(info.imageInfo);

    _manager->updateSets(descriptorSetWrite);
}

void VulkanDescriptorSets::bindPerFrameResource(const VulkanDescriptorInfo& info) const {
    for (uint32_t i = 0; i < _manager->getFramesInFlight(); i++) {
        bindResource(info, i);
    }
}

void VulkanDescriptorSets::bindPerFrameUBO(const VulkanUniformBufferBase& ubo, const uint32_t binding) const {
    for (uint32_t i = 0; i < _manager->getFramesInFlight(); i++) {
        bindResource(ubo.getDescriptorInfo(binding, i), i);
    }
}
