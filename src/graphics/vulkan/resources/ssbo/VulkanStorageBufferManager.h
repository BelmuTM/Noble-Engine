#pragma once

#include "VulkanStorageBuffer.h"

class VulkanStorageBufferManager {
public:
    VulkanStorageBufferManager()  = default;
    ~VulkanStorageBufferManager() = default;

    VulkanStorageBufferManager(const VulkanStorageBufferManager&)            = delete;
    VulkanStorageBufferManager& operator=(const VulkanStorageBufferManager&) = delete;

    VulkanStorageBufferManager(VulkanStorageBufferManager&&)            = delete;
    VulkanStorageBufferManager& operator=(VulkanStorageBufferManager&&) = delete;

    [[nodiscard]] bool create(
        const VulkanDevice& device,
        std::uint32_t       framesInFlight,
        std::string&        errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] VulkanStorageBuffer* allocateBuffer(vk::DeviceSize size, std::string& errorMessage);

private:
    const VulkanDevice* _device = nullptr;

    std::uint32_t _framesInFlight = 0;

    std::vector<std::unique_ptr<VulkanStorageBuffer>> _storageBuffers{};
};

