#pragma once

#include "VulkanUniformBuffer.h"

class VulkanUniformBufferManager {
public:
    VulkanUniformBufferManager()  = default;
    ~VulkanUniformBufferManager() = default;

    VulkanUniformBufferManager(const VulkanUniformBufferManager&)            = delete;
    VulkanUniformBufferManager& operator=(const VulkanUniformBufferManager&) = delete;

    VulkanUniformBufferManager(VulkanUniformBufferManager&&)            = delete;
    VulkanUniformBufferManager& operator=(VulkanUniformBufferManager&&) = delete;

    [[nodiscard]] bool create(
        const VulkanDevice& device,
        std::uint32_t       framesInFlight,
        std::string&        errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] bool createBuffer(VulkanUniformBufferBase& buffer, std::string& errorMessage);

private:
    const VulkanDevice* _device = nullptr;

    std::uint32_t _framesInFlight = 0;

    std::vector<VulkanUniformBufferBase*> _uniformBuffers{};
};
