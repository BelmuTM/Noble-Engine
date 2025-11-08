#include "VulkanDescriptorManager.h"

#include "core/Engine.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

bool VulkanDescriptorManager::create(
    const vk::Device& device,
    const uint32_t    framesInFlight,
    std::string&      errorMessage
) noexcept {
    _device         = device;
    _framesInFlight = framesInFlight;
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

    _device = VK_NULL_HANDLE;
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
        .setMaxSets(MAX_FRAMES_IN_FLIGHT * MAX_OBJECTS)
        .setPoolSizes(poolSizes);

    VK_CREATE(_device.createDescriptorPool(descriptorPoolInfo), _descriptorPool, errorMessage);

    return true;
}

bool VulkanDescriptorManager::allocateSets(
    std::vector<vk::DescriptorSet>&      descriptorSets,
    const vk::DescriptorSetAllocateInfo& descriptorSetInfo,
    std::string&                         errorMessage
) const {
    if (!_device) {
        errorMessage = "Failed to allocate Vulkan descriptor sets: device is null";
        return false;
    }

    VK_CREATE(_device.allocateDescriptorSets(descriptorSetInfo), descriptorSets, errorMessage);

    return true;
}

void VulkanDescriptorManager::updateSets(const vk::WriteDescriptorSet& descriptorSetWrite) const {
    if (!_device) return;

    _device.updateDescriptorSets(descriptorSetWrite, {});
}
