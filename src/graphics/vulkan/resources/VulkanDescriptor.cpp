#include "VulkanDescriptor.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

bool VulkanDescriptor::create(
    const vk::Device& device,
    const uint32_t    framesInFlight,
    std::string&      errorMessage
) noexcept {
    _device         = device;
    _framesInFlight = framesInFlight;
    return true;
}

bool VulkanDescriptor::createSetLayout(
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

bool VulkanDescriptor::createPool(const vk::DescriptorType type, std::string& errorMessage) {
    if (!_device) {
        errorMessage = "Failed to create Vulkan descriptor pool: device is null";
        return false;
    }

    vk::DescriptorPoolSize descriptorPoolSize(type, _framesInFlight);

    vk::DescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo
        .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
        .setMaxSets(_framesInFlight)
        .setPoolSizes(descriptorPoolSize);

    VK_CREATE(_device.createDescriptorPool(descriptorPoolInfo), _descriptorPool, errorMessage);

    return true;
}

bool VulkanDescriptor::allocateSets(std::string& errorMessage) {
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

void VulkanDescriptor::destroy() noexcept {
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

void VulkanDescriptor::updateSet(
    const uint32_t                  frameIndex,
    const uint32_t                  binding,
    const vk::DescriptorType        type,
    const vk::DescriptorBufferInfo& bufferInfo
) const {
    if (!_device) return;

    assert(frameIndex < _descriptorSets.size());

    vk::WriteDescriptorSet descriptorSetWrite{};
    descriptorSetWrite
        .setDstSet(_descriptorSets[frameIndex])
        .setDstBinding(binding)
        .setDstArrayElement(0)
        .setDescriptorCount(1)
        .setDescriptorType(type)
        .setBufferInfo(bufferInfo);

    _device.updateDescriptorSets(descriptorSetWrite, {});
}
