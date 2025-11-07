#pragma once
#ifndef NOBLEENGINE_VULKANDESCRIPTORMANAGER_H
#define NOBLEENGINE_VULKANDESCRIPTORMANAGER_H

#include "graphics/vulkan/common/VulkanHeader.h"

class VulkanUniformBufferBase;

class VulkanDescriptorManager {
public:
    VulkanDescriptorManager()  = default;
    ~VulkanDescriptorManager() = default;

    VulkanDescriptorManager(const VulkanDescriptorManager&)            = delete;
    VulkanDescriptorManager& operator=(const VulkanDescriptorManager&) = delete;

    VulkanDescriptorManager(VulkanDescriptorManager&&)            = delete;
    VulkanDescriptorManager& operator=(VulkanDescriptorManager&&) = delete;

    [[nodiscard]] bool create(const vk::Device& device, uint32_t framesInFlight, std::string& errorMessage) noexcept;

    void destroy() noexcept;

    [[nodiscard]] bool createSetLayout(
        const std::vector<vk::DescriptorSetLayoutBinding>& bindings, std::string& errorMessage
    );

    [[nodiscard]] bool createPool(const std::vector<vk::DescriptorPoolSize>& poolSizes, std::string& errorMessage);

    [[nodiscard]] bool allocateSets(
        std::vector<vk::DescriptorSet>&      descriptorSets,
        const vk::DescriptorSetAllocateInfo& descriptorSetInfo,
        std::string&                         errorMessage
    ) const;

    void updateSets(const vk::WriteDescriptorSet& descriptorSetWrite) const;

    [[nodiscard]] uint32_t getFramesInFlight() const noexcept { return _framesInFlight; }

    [[nodiscard]] vk::DescriptorSetLayout getLayout() const noexcept { return _descriptorSetLayout; }

    [[nodiscard]] vk::DescriptorPool getPool() const noexcept { return _descriptorPool; }

private:
    vk::Device _device{};

    uint32_t _framesInFlight = 0;

    vk::DescriptorSetLayout _descriptorSetLayout{};
    vk::DescriptorPool      _descriptorPool{};
};

#endif //NOBLEENGINE_VULKANDESCRIPTORMANAGER_H
