#pragma once
#ifndef NOBLEENGINE_VULKANBUFFER_H
#define NOBLEENGINE_VULKANBUFFER_H

#include "graphics/vulkan/common/VulkanHeader.h"
#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/core/VulkanCommandManager.h"

class VulkanBuffer {
public:
    VulkanBuffer()  = default;
    ~VulkanBuffer() = default;

    // Implicit conversion operators
    operator vk::Buffer() const { return _buffer; }

    operator       vk::Buffer&()       { return _buffer; }
    operator const vk::Buffer&() const { return _buffer; }

    VulkanBuffer(const VulkanBuffer&)            = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;

    VulkanBuffer(VulkanBuffer&&) noexcept;
    VulkanBuffer& operator=(VulkanBuffer&&) noexcept;

    [[nodiscard]] bool create(
        vk::DeviceSize          size,
        vk::BufferUsageFlags    usage,
        vk::MemoryPropertyFlags properties,
        const VulkanDevice*     device,
        std::string&            errorMessage
    ) noexcept;

    void destroy() noexcept;

    static bool createBuffer(
        vk::Buffer&             buffer,
        vk::DeviceMemory&       bufferMemory,
        vk::DeviceSize          size,
        vk::BufferUsageFlags    usage,
        vk::MemoryPropertyFlags properties,
        const VulkanDevice*     device,
        std::string&            errorMessage
    );

    static bool copyBuffer(
        const vk::Buffer&           srcBuffer,
        const vk::Buffer&           dstBuffer,
        vk::DeviceSize              size,
        vk::DeviceSize              srcOffset,
        vk::DeviceSize              dstOffset,
        const VulkanDevice*         device,
        const VulkanCommandManager* commandManager,
        std::string&                errorMessage
    );

    bool copyFrom(
        const vk::Buffer&           srcBuffer,
        const VulkanCommandManager* commandManager,
        std::string&                errorMessage,
        vk::DeviceSize              size      = VK_WHOLE_SIZE,
        vk::DeviceSize              srcOffset = 0,
        vk::DeviceSize              dstOffset = 0
    ) const;

    void* mapMemory(std::string& errorMessage, vk::DeviceSize offset = 0, vk::DeviceSize size = VK_WHOLE_SIZE);
    void unmapMemory();

    [[nodiscard]] void* getMappedPointer() const { return _mappedPointer; }

private:
    const VulkanDevice* _device = nullptr;

    vk::Buffer       _buffer{};
    vk::DeviceSize   _bufferSize{};
    vk::DeviceMemory _bufferMemory{};

    void* _mappedPointer = nullptr;
};

#endif // NOBLEENGINE_VULKANBUFFER_H
