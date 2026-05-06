#pragma once

#include "core/debug/ErrorHandling.h"

#include "VulkanUniformBuffer.h"

class VulkanUniformBufferManager {
public:
    VulkanUniformBufferManager()  = default;
    ~VulkanUniformBufferManager() = default;

    VulkanUniformBufferManager(const VulkanUniformBufferManager&)            = delete;
    VulkanUniformBufferManager& operator=(const VulkanUniformBufferManager&) = delete;

    VulkanUniformBufferManager(VulkanUniformBufferManager&&)            = delete;
    VulkanUniformBufferManager& operator=(VulkanUniformBufferManager&&) = delete;

    [[nodiscard]] Expected<void> create(const VulkanDevice& device, std::uint32_t framesInFlight) noexcept;

    void destroy() noexcept;

    template<typename UniformBufferType>
    [[nodiscard]] Expected<UniformBufferType*> allocateBuffer() {
        _uniformBuffers.push_back(std::make_unique<UniformBufferType>());

        TRY(_uniformBuffers.back()->create(*_device, _framesInFlight));

        return Expected<UniformBufferType*>(
            static_cast<UniformBufferType*>(_uniformBuffers.back().get())
        );
    }

private:
    const VulkanDevice* _device = nullptr;

    std::uint32_t _framesInFlight = 0;

    std::vector<std::unique_ptr<VulkanUniformBufferBase>> _uniformBuffers{};
};
