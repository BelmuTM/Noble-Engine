#include "VulkanDescriptorManager.h"

#include "graphics/vulkan/common/VulkanDebugger.h"
#include "VulkanDescriptorSets.h"

#include "core/debug/ErrorHandling.h"

bool VulkanDescriptorManager::create(
    const vk::Device&             device,
    const VulkanDescriptorScheme& descriptorScheme,
    const uint32_t                framesInFlight,
    const uint32_t                maxSets,
    std::string&                  errorMessage
) noexcept {
    _device         = device;
    _framesInFlight = framesInFlight;
    _maxSets        = framesInFlight * maxSets; // 1 set per frame in flight

    buildDescriptorScheme(descriptorScheme);

    TRY(createSetLayout(errorMessage));
    TRY(createPool(errorMessage));

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

bool VulkanDescriptorManager::allocate(VulkanDescriptorSets*& descriptorSets, std::string& errorMessage) {
    VulkanDescriptorSets tempDescriptorSets{*this};

    TRY(tempDescriptorSets.allocate(errorMessage));

    _descriptorSets.push_back(std::make_unique<VulkanDescriptorSets>(std::move(tempDescriptorSets)));
    descriptorSets = _descriptorSets.back().get();

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

void VulkanDescriptorManager::buildDescriptorScheme(const VulkanDescriptorScheme& descriptorScheme) {
    _bindings.reserve(descriptorScheme.size());
    _poolSizes.reserve(descriptorScheme.size());

    for (const auto& [binding, type, stageFlags, count, name] : descriptorScheme) {
        _bindings.emplace_back(binding, type, count, stageFlags, nullptr);
        _poolSizes.emplace_back(type, count * _maxSets);
    }
}

bool VulkanDescriptorManager::createSetLayout(std::string& errorMessage) {
    if (!_device) {
        errorMessage = "Failed to create Vulkan descriptor set layout: device is null";
        return false;
    }

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.setBindings(_bindings);

    VK_CREATE(_device.createDescriptorSetLayout(descriptorSetLayoutInfo), _descriptorSetLayout, errorMessage);

    return true;
}

bool VulkanDescriptorManager::createPool(std::string& errorMessage) {
    if (!_device) {
        errorMessage = "Failed to create Vulkan descriptor pool: device is null";
        return false;
    }

    vk::DescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo
        .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
        .setMaxSets(_maxSets)
        .setPoolSizes(_poolSizes);

    VK_CREATE(_device.createDescriptorPool(descriptorPoolInfo), _descriptorPool, errorMessage);

    return true;
}
