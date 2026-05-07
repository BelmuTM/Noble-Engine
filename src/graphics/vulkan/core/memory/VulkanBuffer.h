#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/core/VulkanCommandManager.h"

#include "graphics/vulkan/core/memory/VmaUsage.h"

#include <cstring>

class VulkanBuffer {
public:
    VulkanBuffer()  = default;
    ~VulkanBuffer() = default;

    VulkanBuffer(const VulkanBuffer&)            = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;

    VulkanBuffer(VulkanBuffer&&)            noexcept;
    VulkanBuffer& operator=(VulkanBuffer&&) noexcept;

    [[nodiscard]] Expected<void> create(
        vk::DeviceSize       size,
        vk::BufferUsageFlags usage,
        VmaMemoryUsage       memoryUsage,
        const VulkanDevice*  device
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] static Expected<void> createBuffer(
        vk::Buffer&          buffer,
        VmaAllocation&       allocation,
        vk::DeviceSize       size,
        vk::BufferUsageFlags usage,
        VmaMemoryUsage       memoryUsage,
        const VulkanDevice*  device
    );

    [[nodiscard]] static Expected<void> copyBuffer(
        const vk::Buffer&           srcBuffer,
        const vk::Buffer&           dstBuffer,
        vk::DeviceSize              size,
        vk::DeviceSize              srcOffset,
        vk::DeviceSize              dstOffset,
        const VulkanCommandManager* commandManager
    );

    [[nodiscard]] Expected<void> copyFrom(
        const vk::Buffer&           srcBuffer,
        const VulkanCommandManager* commandManager,
        vk::DeviceSize              size      = VK_WHOLE_SIZE,
        vk::DeviceSize              srcOffset = 0,
        vk::DeviceSize              dstOffset = 0
    ) const;

    // Map GPU allocated memory to CPU memory
    Expected<void> mapMemory();

    void unmapMemory();

    void updateMemory(
        const void*          data,
        const vk::DeviceSize size   = VK_WHOLE_SIZE,
        const vk::DeviceSize offset = 0
    ) const {
        // Using persistent mapping to update memory in place
        std::memcpy(static_cast<std::byte*>(_mappedPointer) + offset, data, size);
    }

    template<typename BufferDataType>
    requires (!std::is_pointer_v<BufferDataType>)
    void updateMemory(const BufferDataType& data, const vk::DeviceSize offset = 0) const {
        updateMemory(&data, sizeof(BufferDataType), offset);
    }

    template<typename BufferDataType>
    void updateArrayMemory(
        const BufferDataType* data,
        const std::size_t     count,
        const vk::DeviceSize  offset = 0
    ) const {
        updateMemory(data, count * sizeof(BufferDataType), offset);
    }

    static vk::DeviceSize align(const vk::DeviceSize size, const vk::DeviceSize alignment) noexcept {
        return (size + alignment - 1) & ~(alignment - 1);
    }

    [[nodiscard]] vk::Buffer handle() const noexcept { return _buffer; }

    [[nodiscard]] vk::DeviceAddress getDeviceAddress() const noexcept { return _deviceAddress; }

    [[nodiscard]] void* getMappedPointer() const noexcept { return _mappedPointer; }

private:
    const VulkanDevice* _device = nullptr;

    vk::Buffer     _buffer{};
    vk::DeviceSize _bufferSize = 0;

    vk::DeviceAddress _deviceAddress = 0;

    VmaAllocation _allocation = VK_NULL_HANDLE;

    void* _mappedPointer = nullptr;
};
