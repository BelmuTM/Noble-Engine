#include "VulkanDescriptorManager.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "VulkanDescriptorSets.h"

Expected<void> VulkanDescriptorManager::create(
    const vk::Device&             device,
    const VulkanDescriptorScheme& descriptorScheme,
    const std::uint32_t           framesInFlight,
    const std::uint32_t           setCount
) noexcept {
    _device         = device;
    _framesInFlight = framesInFlight;
    _setCount       = framesInFlight * setCount; // 1 set per frame in flight

    buildDescriptorScheme(descriptorScheme);

    TRY(createSetLayout());
    TRY(createPool());

    return {};
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

Expected<void> VulkanDescriptorManager::allocate(VulkanDescriptorSets*& descriptorSets) {
    VulkanDescriptorSets tempDescriptorSets{*this};

    TRY(tempDescriptorSets.allocate());

    _descriptorSets.push_back(std::make_unique<VulkanDescriptorSets>(std::move(tempDescriptorSets)));

    descriptorSets = _descriptorSets.back().get();

    return {};
}

Expected<void> VulkanDescriptorManager::allocateSets(
    std::vector<vk::DescriptorSet>&      descriptorSets,
    const vk::DescriptorSetAllocateInfo& descriptorSetInfo
) const {
    if (!_device) {
        return VK_FAIL("Failed to allocate descriptor sets: device is null.");
    }

    VK_CREATE(descriptorSets, _device.allocateDescriptorSets(descriptorSetInfo));

    return {};
}

void VulkanDescriptorManager::updateSets(const vk::WriteDescriptorSet& descriptorSetWrite) const {
    if (!_device) return;

    _device.updateDescriptorSets(descriptorSetWrite, {});
}

void VulkanDescriptorManager::buildDescriptorScheme(const VulkanDescriptorScheme& descriptorScheme) {
    _bindings.reserve(descriptorScheme.size());
    _poolSizes.reserve(descriptorScheme.size());

    for (const auto& [binding, type, stageFlags, count] : descriptorScheme) {
        _bindings.emplace_back(binding, type, count, stageFlags, nullptr);
        _poolSizes.emplace_back(type, count * _setCount);
    }
}

Expected<void> VulkanDescriptorManager::createSetLayout() {
    if (!_device) {
        return VK_FAIL("Failed to create descriptor set layout: device is null.");
    }

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.setBindings(_bindings);

    VK_CREATE(_descriptorSetLayout, _device.createDescriptorSetLayout(descriptorSetLayoutInfo));

    return {};
}

Expected<void> VulkanDescriptorManager::createPool() {
    if (!_device) {
        return VK_FAIL("Failed to create descriptor pool: device is null.");
    }

    vk::DescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo
        .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
        .setMaxSets(_setCount)
        .setPoolSizes(_poolSizes);

    VK_CREATE(_descriptorPool, _device.createDescriptorPool(descriptorPoolInfo));

    return {};
}
