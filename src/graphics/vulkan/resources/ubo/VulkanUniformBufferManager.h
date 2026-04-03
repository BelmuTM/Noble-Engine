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

    template<typename UniformBufferType>
    [[nodiscard]] UniformBufferType* allocateBuffer(std::string& errorMessage) {
        _uniformBuffers.push_back(std::make_unique<UniformBufferType>());

        if (!_uniformBuffers.back()->create(*_device, _framesInFlight, errorMessage)) {
            return nullptr;
        }

        return static_cast<UniformBufferType*>(_uniformBuffers.back().get());
    }

private:
    const VulkanDevice* _device = nullptr;

    std::uint32_t _framesInFlight = 0;

    std::vector<std::unique_ptr<VulkanUniformBufferBase>> _uniformBuffers{};
};
