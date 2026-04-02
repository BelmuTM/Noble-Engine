#pragma once

#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/core/memory/VulkanBuffer.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorInfo.h"

#include "VulkanInstanceHandle.h"

class VulkanInstanceHandleBuffer final : public VulkanBuffer {
public:
    VulkanInstanceHandleBuffer()  = default;
    ~VulkanInstanceHandleBuffer() = default;

    VulkanInstanceHandleBuffer(const VulkanInstanceHandleBuffer&)            = delete;
    VulkanInstanceHandleBuffer& operator=(const VulkanInstanceHandleBuffer&) = delete;

    VulkanInstanceHandleBuffer(VulkanInstanceHandleBuffer&&)            = delete;
    VulkanInstanceHandleBuffer& operator=(VulkanInstanceHandleBuffer&&) = delete;

    [[nodiscard]] bool create(const VulkanDevice& device, std::uint32_t maxInstances, std::string& errorMessage) noexcept;

    void destroy() noexcept;

    void update(const std::vector<VulkanInstanceHandle>& handles) const;

    [[nodiscard]] VulkanDescriptorInfo getDescriptorInfo(const std::uint32_t binding) const noexcept {
        return {
            .type       = vk::DescriptorType::eStorageBuffer,
            .bufferInfo = {handle(), 0, VK_WHOLE_SIZE},
            .binding    = binding
        };
    }

private:
    const VulkanDevice* _device = nullptr;

    std::uint32_t _maxInstances = 0;
};
