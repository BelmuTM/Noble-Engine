#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/core/memory/VulkanBuffer.h"
#include "graphics/vulkan/resources/descriptors/VulkanDescriptorInfo.h"

class VulkanStorageBuffer {
public:
    VulkanStorageBuffer()  = default;
    ~VulkanStorageBuffer() = default;

    VulkanStorageBuffer(const VulkanStorageBuffer&)            = delete;
    VulkanStorageBuffer& operator=(const VulkanStorageBuffer&) = delete;

    VulkanStorageBuffer(VulkanStorageBuffer&&)            = delete;
    VulkanStorageBuffer& operator=(VulkanStorageBuffer&&) = delete;

    [[nodiscard]] bool create(
        const VulkanDevice& device,
        std::uint32_t       framesInFlight,
        vk::DeviceSize      size,
        std::string&        errorMessage
    ) noexcept;

    void destroy() noexcept;

    void updateMemory(
        const std::uint32_t  frameIndex,
        const void*          data,
        const vk::DeviceSize size,
        const vk::DeviceSize offset = 0
    ) const {
        _storageBuffers[frameIndex].updateMemory(data, size, offset);
    }

    template<typename StorageBufferType>
    void updateMemory(
        const uint32_t           frameIndex,
        const StorageBufferType& data,
        const vk::DeviceSize     offset = 0
    ) const {
        _storageBuffers[frameIndex].updateMemory(data, offset);
    }

    template<typename StorageBufferType>
    void updateArrayMemory(
        const uint32_t           frameIndex,
        const StorageBufferType* data,
        const size_t             count,
        const vk::DeviceSize     offset = 0
    ) const {
        _storageBuffers[frameIndex].updateArrayMemory(data, count, offset);
    }

    [[nodiscard]] VulkanDescriptorInfo getDescriptorInfo(
        const std::uint32_t binding, const std::uint32_t frameIndex
    ) const noexcept {
        return {
            .type       = vk::DescriptorType::eStorageBuffer,
            .bufferInfo = {_storageBuffers[frameIndex].handle(), 0, getBufferSize()},
            .binding    = binding
        };
    }

    [[nodiscard]]       std::vector<VulkanBuffer>& getBuffers()       noexcept { return _storageBuffers; }
    [[nodiscard]] const std::vector<VulkanBuffer>& getBuffers() const noexcept { return _storageBuffers; }

protected:
    bool createStorageBuffers(std::string& errorMessage);

    [[nodiscard]] vk::DeviceSize getBufferSize() const noexcept {
        return VulkanBuffer::align(_bufferSize, _device->getLimits().minStorageBufferOffsetAlignment);
    }

    const VulkanDevice* _device = nullptr;

    std::uint32_t _framesInFlight = 0;

    vk::DeviceSize _bufferSize = 0;

    std::vector<VulkanBuffer> _storageBuffers{};
};
