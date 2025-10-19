#pragma once
#ifndef NOBLEENGINE_VULKANDESCRIPTORMANAGER_H
#define NOBLEENGINE_VULKANDESCRIPTORMANAGER_H

#include "graphics/vulkan/common/VulkanHeader.h"
#include "graphics/vulkan/core/memory/VulkanBuffer.h"

struct DescriptorInfo {
    vk::DescriptorType       type;
    vk::DescriptorImageInfo  imageInfo{};
    vk::DescriptorBufferInfo bufferInfo{};
    uint32_t                 binding;
};

class VulkanUniformBufferBase;

class VulkanDescriptorManager {
public:
    VulkanDescriptorManager()  = default;
    ~VulkanDescriptorManager() = default;

    VulkanDescriptorManager(const VulkanDescriptorManager&)            = delete;
    VulkanDescriptorManager& operator=(const VulkanDescriptorManager&) = delete;
    VulkanDescriptorManager(VulkanDescriptorManager&&)                 = delete;
    VulkanDescriptorManager& operator=(VulkanDescriptorManager&&)      = delete;

    [[nodiscard]] bool create(const vk::Device& device, uint32_t framesInFlight, std::string& errorMessage) noexcept;

    [[nodiscard]] bool createSetLayout(
        const std::vector<vk::DescriptorSetLayoutBinding>& bindings, std::string& errorMessage
    );
    [[nodiscard]] bool createPool(const std::vector<vk::DescriptorPoolSize>& poolSizes, std::string& errorMessage);
    [[nodiscard]] bool allocateSets(std::string& errorMessage);

    void destroy() noexcept;

    [[nodiscard]] vk::DescriptorSetLayout getLayout() const { return _descriptorSetLayout; }

    [[nodiscard]] const std::vector<vk::DescriptorSet>& getDescriptorSets() const { return _descriptorSets; }

    void bindResource(const DescriptorInfo& info, uint32_t frameIndex) const;

    void bindPerFrameResource(const DescriptorInfo& info) const;

    void bindPerFrameUBO(const VulkanUniformBufferBase& ubo, uint32_t binding) const;

private:
    vk::Device _device{};

    uint32_t _framesInFlight = 0;

    vk::DescriptorSetLayout        _descriptorSetLayout{};
    vk::DescriptorPool             _descriptorPool{};
    std::vector<vk::DescriptorSet> _descriptorSets{};
};

#endif //NOBLEENGINE_VULKANDESCRIPTORMANAGER_H
