#pragma once
#ifndef NOBLEENGINE_VULKANOBJECTBUFFER_H
#define NOBLEENGINE_VULKANOBJECTBUFFER_H

#include "graphics/vulkan/common/VulkanHeader.h"
#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/core/memory/VulkanBuffer.h"

#include "core/objects/object/Object.h"

#include <glm/glm.hpp>

class VulkanObjectBuffer {
public:
    VulkanObjectBuffer()  = default;
    ~VulkanObjectBuffer() = default;

    VulkanObjectBuffer(const VulkanObjectBuffer&)            = delete;
    VulkanObjectBuffer& operator=(const VulkanObjectBuffer&) = delete;

    VulkanObjectBuffer(VulkanObjectBuffer&&)            = delete;
    VulkanObjectBuffer& operator=(VulkanObjectBuffer&&) = delete;

    [[nodiscard]] bool create(const VulkanDevice& device, uint32_t maxObjects, std::string& errorMessage) noexcept;

    void destroy() noexcept;

    void update(uint32_t objectIndex, const ObjectDataGPU& data) const;

    [[nodiscard]] VulkanBuffer& getBuffer() noexcept { return _buffer; }

private:
    const VulkanDevice* _device = nullptr;

    VulkanBuffer _buffer;

    uint32_t _maxObjects;
};

#endif // NOBLEENGINE_VULKANOBJECTBUFFER_H
