#pragma once
#ifndef NOBLEENGINE_VULKANBUFFER_H
#define NOBLEENGINE_VULKANBUFFER_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/core/VulkanCommandManager.h"

#include "graphics/vulkan/core/memory/VmaUsage.h"

class VulkanBuffer {
public:
    VulkanBuffer()  = default;
    ~VulkanBuffer() = default;

    // Implicit conversion operators
    operator vk::Buffer() const { return _buffer; }

    VulkanBuffer(const VulkanBuffer&)            = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;

    VulkanBuffer(VulkanBuffer&&)            noexcept;
    VulkanBuffer& operator=(VulkanBuffer&&) noexcept;

    [[nodiscard]] bool create(
        vk::DeviceSize       size,
        vk::BufferUsageFlags usage,
        VmaMemoryUsage       memoryUsage,
        const VulkanDevice*  device,
        std::string&         errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] static bool createBuffer(
        vk::Buffer&          buffer,
        VmaAllocation&       allocation,
        vk::DeviceSize       size,
        vk::BufferUsageFlags usage,
        VmaMemoryUsage       memoryUsage,
        const VulkanDevice*  device,
        std::string&         errorMessage
    );

    [[nodiscard]] static bool copyBuffer(
        const vk::Buffer&           srcBuffer,
        const vk::Buffer&           dstBuffer,
        vk::DeviceSize              size,
        vk::DeviceSize              srcOffset,
        vk::DeviceSize              dstOffset,
        const VulkanCommandManager* commandManager,
        std::string&                errorMessage
    );

    [[nodiscard]] bool copyFrom(
        const vk::Buffer&           srcBuffer,
        const VulkanCommandManager* commandManager,
        std::string&                errorMessage,
        vk::DeviceSize              size      = VK_WHOLE_SIZE,
        vk::DeviceSize              srcOffset = 0,
        vk::DeviceSize              dstOffset = 0
    ) const;

    // Mapping GPU allocated memory to CPU memory
    void* mapMemory(std::string& errorMessage);
    void unmapMemory();

    [[nodiscard]] vk::DeviceAddress getDeviceAddress() const noexcept { return _deviceAddress; }

    [[nodiscard]] void* getMappedPointer() const noexcept { return _mappedPointer; }

private:
    const VulkanDevice* _device = nullptr;

    vk::Buffer     _buffer{};
    vk::DeviceSize _bufferSize;

    vk::DeviceAddress _deviceAddress;

    VmaAllocation _allocation;

    void* _mappedPointer = nullptr;
};

#endif // NOBLEENGINE_VULKANBUFFER_H
