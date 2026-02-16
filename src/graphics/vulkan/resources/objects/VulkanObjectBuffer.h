#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/core/memory/VulkanBuffer.h"
#include "graphics/vulkan/core/VulkanDevice.h"

#include "core/entities/objects/Object.h"

class VulkanObjectBuffer final : public VulkanBuffer {
public:
    VulkanObjectBuffer()  = default;
    ~VulkanObjectBuffer() = default;

    VulkanObjectBuffer(const VulkanObjectBuffer&)            = delete;
    VulkanObjectBuffer& operator=(const VulkanObjectBuffer&) = delete;

    VulkanObjectBuffer(VulkanObjectBuffer&&)            = delete;
    VulkanObjectBuffer& operator=(VulkanObjectBuffer&&) = delete;

    [[nodiscard]] bool create(const VulkanDevice& device, uint32_t maxObjects, std::string& errorMessage) noexcept;

    void destroy() noexcept;

    void update(uint32_t objectIndex, const ObjectDataGPU& dataToGPU) const;

    void update(const std::vector<ObjectDataGPU>& dataToGPU) const;

private:
    const VulkanDevice* _device = nullptr;

    uint32_t _maxObjects = 0;
};
