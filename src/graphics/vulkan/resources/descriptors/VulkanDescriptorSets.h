#pragma once
#ifndef NOBLEENGINE_VULKANDESCRIPTORSETS_H
#define NOBLEENGINE_VULKANDESCRIPTORSETS_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/resources/ubo/VulkanUniformBuffer.h"
#include "VulkanDescriptorManager.h"
#include "VulkanDescriptorInfo.h"

#include <vector>

class VulkanDescriptorSets {
public:
    explicit VulkanDescriptorSets(const VulkanDescriptorManager& descriptorManager) : _manager(&descriptorManager) {}

    ~VulkanDescriptorSets() = default;

    VulkanDescriptorSets(const VulkanDescriptorSets&)            noexcept = default;
    VulkanDescriptorSets& operator=(const VulkanDescriptorSets&) noexcept = default;

    VulkanDescriptorSets(VulkanDescriptorSets&&)            noexcept = default;
    VulkanDescriptorSets& operator=(VulkanDescriptorSets&&) noexcept = default;

    [[nodiscard]] bool allocate(std::string& errorMessage);

    void bindResource(const VulkanDescriptorInfo& info, uint32_t frameIndex) const;

    void bindPerFrameResource(const VulkanDescriptorInfo& info) const;

    void bindPerFrameUBO(const VulkanUniformBufferBase& ubo, uint32_t binding) const;

    [[nodiscard]] std::vector<vk::DescriptorSet> getSets() const noexcept { return _descriptorSets; }

private:
    const VulkanDescriptorManager* _manager = nullptr;

    std::vector<vk::DescriptorSet> _descriptorSets{};
};

#endif // NOBLEENGINE_VULKANDESCRIPTORSETS_H
