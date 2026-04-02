#pragma once

#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/core/memory/VulkanBuffer.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorInfo.h"

#include "core/entities/objects/Object.h"

class VulkanObjectBuffer final : public VulkanBuffer {
public:
    VulkanObjectBuffer()  = default;
    ~VulkanObjectBuffer() = default;

    VulkanObjectBuffer(const VulkanObjectBuffer&)            = delete;
    VulkanObjectBuffer& operator=(const VulkanObjectBuffer&) = delete;

    VulkanObjectBuffer(VulkanObjectBuffer&&)            = delete;
    VulkanObjectBuffer& operator=(VulkanObjectBuffer&&) = delete;

    [[nodiscard]] bool create(const VulkanDevice& device, std::uint32_t maxObjects, std::string& errorMessage) noexcept;

    void destroy() noexcept;

    void update(std::uint32_t objectIndex, const ObjectDataGPU& dataToGPU) const;

    void update(const std::vector<ObjectDataGPU>& dataToGPU) const;

    [[nodiscard]] VulkanDescriptorInfo getDescriptorInfo(const std::uint32_t binding) const noexcept {
        return {
            .type       = vk::DescriptorType::eStorageBuffer,
            .bufferInfo = {handle(), 0, VK_WHOLE_SIZE},
            .binding    = binding
        };
    }

private:
    const VulkanDevice* _device = nullptr;

    std::uint32_t _maxObjects = 0;
};
