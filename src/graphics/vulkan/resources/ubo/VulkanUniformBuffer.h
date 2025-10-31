#pragma once
#ifndef NOBLEENGINE_VULKANUNIFORMBUFFER_H
#define NOBLEENGINE_VULKANUNIFORMBUFFER_H

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"
#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/core/memory/VulkanBuffer.h"
#include "graphics/vulkan/resources/VulkanDescriptorManager.h"

class VulkanUniformBufferBase {
public:
    virtual DescriptorInfo getDescriptorInfo(uint32_t binding, uint32_t frameIndex) const = 0;
    virtual ~VulkanUniformBufferBase() = default;
};

template<typename T>
class VulkanUniformBuffer : public VulkanUniformBufferBase {
public:
    VulkanUniformBuffer() = default;
    ~VulkanUniformBuffer() override = default;

    VulkanUniformBuffer(const VulkanUniformBuffer&)            = delete;
    VulkanUniformBuffer& operator=(const VulkanUniformBuffer&) = delete;

    VulkanUniformBuffer(VulkanUniformBuffer&&)            = delete;
    VulkanUniformBuffer& operator=(VulkanUniformBuffer&&) = delete;

    [[nodiscard]] bool create(
        const VulkanDevice& device,
        const uint32_t      framesInFlight,
        std::string&        errorMessage
    ) noexcept {
        _device         = &device;
        _framesInFlight = framesInFlight;

        TRY(createUniformBuffers(errorMessage));

        return true;
    }

    void destroy() noexcept {
        for (auto& uniformBuffer : uniformBuffers) {
            uniformBuffer.destroy();
        }
        _device = nullptr;
    }

    DescriptorInfo getDescriptorInfo(const uint32_t binding, const uint32_t frameIndex) const override {
        return {
            .type       = vk::DescriptorType::eUniformBuffer,
            .bufferInfo = {uniformBuffers[frameIndex], 0, size},
            .binding    = binding
        };
    }

    [[nodiscard]] const std::vector<VulkanBuffer>& getBuffers() const { return uniformBuffers; }

protected:
    static constexpr vk::DeviceSize size = sizeof(T);

    const VulkanDevice* _device = nullptr;

    uint32_t _framesInFlight = 0;

    std::vector<VulkanBuffer> uniformBuffers{};

    bool createUniformBuffers(std::string& errorMessage) {
        if (!_device) {
            errorMessage = "Failed to create Vulkan uniform buffers: device is null";
            return false;
        }

        uniformBuffers.clear();
        uniformBuffers.reserve(_framesInFlight);

        for (uint32_t i = 0; i < _framesInFlight; i++) {
            VulkanBuffer uniformBuffer;

            TRY(uniformBuffer.create(
                size,
                vk::BufferUsageFlagBits::eUniformBuffer,
                VMA_MEMORY_USAGE_CPU_TO_GPU,
                _device,
                errorMessage
            ));

            TRY(uniformBuffer.mapMemory(errorMessage));

            uniformBuffers.emplace_back(std::move(uniformBuffer));
        }

        return true;
    }
};

#endif //NOBLEENGINE_VULKANUNIFORMBUFFER_H
