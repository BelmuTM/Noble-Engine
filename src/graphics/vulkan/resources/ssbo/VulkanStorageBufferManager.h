#pragma once

#include "core/debug/ErrorHandling.h"

#include "VulkanStorageBuffer.h"

class VulkanStorageBufferManager {
public:
    VulkanStorageBufferManager()  = default;
    ~VulkanStorageBufferManager() = default;

    VulkanStorageBufferManager(const VulkanStorageBufferManager&)            = delete;
    VulkanStorageBufferManager& operator=(const VulkanStorageBufferManager&) = delete;

    VulkanStorageBufferManager(VulkanStorageBufferManager&&)            = delete;
    VulkanStorageBufferManager& operator=(VulkanStorageBufferManager&&) = delete;

    [[nodiscard]] Expected<void> create(const VulkanDevice& device, std::uint32_t framesInFlight) noexcept;

    void destroy() noexcept;

    [[nodiscard]] Expected<VulkanStorageBuffer*> allocateBuffer(vk::DeviceSize size);

private:
    const VulkanDevice* _device = nullptr;

    std::uint32_t _framesInFlight = 0;

    std::vector<std::unique_ptr<VulkanStorageBuffer>> _storageBuffers{};
};

