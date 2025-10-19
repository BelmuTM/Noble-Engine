#include "VulkanDescriptorManager.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/resources/ubo/VulkanUniformBuffer.h"

bool VulkanDescriptorManager::create(
    const vk::Device& device,
    const uint32_t    framesInFlight,
    std::string&      errorMessage
) noexcept {
    _device         = device;
    _framesInFlight = framesInFlight;
    return true;
}

bool VulkanDescriptorManager::createSetLayout(
    const std::vector<vk::DescriptorSetLayoutBinding>& bindings, std::string& errorMessage
) {
    if (!_device) {
        errorMessage = "Failed to create Vulkan descriptor set layout: device is null";
        return false;
    }

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.setBindings(bindings);

    VK_CREATE(_device.createDescriptorSetLayout(descriptorSetLayoutInfo), _descriptorSetLayout, errorMessage);

    return true;
}

bool VulkanDescriptorManager::createPool(const std::vector<vk::DescriptorPoolSize>& poolSizes, std::string& errorMessage) {
    if (!_device) {
        errorMessage = "Failed to create Vulkan descriptor pool: device is null";
        return false;
    }

    vk::DescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo
        .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
        .setMaxSets(_framesInFlight)
        .setPoolSizes(poolSizes);

    VK_CREATE(_device.createDescriptorPool(descriptorPoolInfo), _descriptorPool, errorMessage);

    return true;
}

bool VulkanDescriptorManager::allocateSets(std::string& errorMessage) {
    if (!_device) {
        errorMessage = "Failed to allocate Vulkan descriptor sets: device is null";
        return false;
    }

    std::vector layouts(_framesInFlight, _descriptorSetLayout);

    vk::DescriptorSetAllocateInfo descriptorSetInfo{};
    descriptorSetInfo
        .setDescriptorPool(_descriptorPool)
        .setSetLayouts(layouts);

    _descriptorSets.clear();
    _descriptorSets.reserve(_framesInFlight);

    VK_CREATE(_device.allocateDescriptorSets(descriptorSetInfo), _descriptorSets, errorMessage);

    return true;
}

void VulkanDescriptorManager::destroy() noexcept {
    if (_descriptorPool && _device) {
        _device.destroyDescriptorPool(_descriptorPool);
        _descriptorPool = VK_NULL_HANDLE;
    }

    if (_descriptorSetLayout && _device) {
        _device.destroyDescriptorSetLayout(_descriptorSetLayout);
        _descriptorSetLayout = VK_NULL_HANDLE;
    }

    _device = nullptr;
}

void VulkanDescriptorManager::bindResource(const DescriptorInfo& info, const uint32_t frameIndex) const {
    if (!_device) return;

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

    _device.updateDescriptorSets(descriptorSetWrite, {});
}

void VulkanDescriptorManager::bindPerFrameResource(const DescriptorInfo& info) const {
    if (!_device) return;

    for (uint32_t i = 0; i < _framesInFlight; i++) {
        bindResource(info, i);
    }
}

void VulkanDescriptorManager::bindPerFrameUBO(const VulkanUniformBufferBase& ubo, const uint32_t binding) const {
    if (!_device) return;

    for (uint32_t i = 0; i < _framesInFlight; i++) {
        bindResource(ubo.getDescriptorInfo(binding, i), i);
    }
}
