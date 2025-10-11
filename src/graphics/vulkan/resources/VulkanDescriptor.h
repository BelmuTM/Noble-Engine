#pragma once
#ifndef NOBLEENGINE_VULKANDESCRIPTOR_H
#define NOBLEENGINE_VULKANDESCRIPTOR_H

#include "graphics/vulkan/common/VulkanHeader.h"
#include "graphics/vulkan/core/memory/VulkanBuffer.h"

class VulkanDescriptor {
public:
    VulkanDescriptor()  = default;
    ~VulkanDescriptor() = default;

    VulkanDescriptor(const VulkanDescriptor&)            = delete;
    VulkanDescriptor& operator=(const VulkanDescriptor&) = delete;
    VulkanDescriptor(VulkanDescriptor&&)                 = delete;
    VulkanDescriptor& operator=(VulkanDescriptor&&)      = delete;

    [[nodiscard]] bool create(const vk::Device& device, uint32_t framesInFlight, std::string& errorMessage) noexcept;

    [[nodiscard]] bool createSetLayout(
        const std::vector<vk::DescriptorSetLayoutBinding>& bindings, std::string& errorMessage
    );
    [[nodiscard]] bool createPool(const std::vector<vk::DescriptorPoolSize>& poolSizes, std::string& errorMessage);
    [[nodiscard]] bool allocateSets(std::string& errorMessage);

    void destroy() noexcept;

    [[nodiscard]] vk::DescriptorSetLayout getLayout() const { return _descriptorSetLayout; }

    void updateSet(
        uint32_t                        frameIndex,
        uint32_t                        binding,
        vk::DescriptorType              type,
        const vk::DescriptorBufferInfo& bufferInfo
    ) const;

    void updateSets(
        uint32_t                       binding,
        vk::DescriptorType             type,
        const vk::DescriptorImageInfo& imageInfo
    ) const;

    [[nodiscard]] const std::vector<vk::DescriptorSet>& getDescriptorSets() const { return _descriptorSets; }

private:
    vk::Device _device{};

    uint32_t _framesInFlight = 0;

    vk::DescriptorSetLayout        _descriptorSetLayout{};
    vk::DescriptorPool             _descriptorPool{};
    std::vector<vk::DescriptorSet> _descriptorSets{};
};

#endif //NOBLEENGINE_VULKANDESCRIPTOR_H
