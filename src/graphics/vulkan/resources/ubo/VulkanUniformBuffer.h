#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/core/memory/VulkanBuffer.h"
#include "graphics/vulkan/resources/descriptors/VulkanDescriptorInfo.h"

#include "core/debug/ErrorHandling.h"

class VulkanUniformBufferBase {
public:
    virtual ~VulkanUniformBufferBase() = default;

    [[nodiscard]] virtual bool create(
        const VulkanDevice& device,
        uint32_t            framesInFlight,
        std::string&        errorMessage
    ) noexcept = 0;

    virtual void destroy() noexcept = 0;

    [[nodiscard]] virtual VulkanDescriptorInfo getDescriptorInfo(uint32_t binding, uint32_t frameIndex) const = 0;
};

template <typename UniformBufferType>
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
    ) noexcept override {
        _device         = &device;
        _framesInFlight = framesInFlight;

        TRY_deprecated(createUniformBuffers(errorMessage));

        return true;
    }

    void destroy() noexcept override {
        for (auto& uniformBuffer : uniformBuffers) {
            uniformBuffer.destroy();
        }

        _device = nullptr;
    }

    [[nodiscard]] VulkanDescriptorInfo getDescriptorInfo(
        const uint32_t binding, const uint32_t frameIndex
    ) const noexcept override {
        return {
            .type       = vk::DescriptorType::eUniformBuffer,
            .bufferInfo = {uniformBuffers[frameIndex].handle(), 0, BUFFER_SIZE},
            .binding    = binding
        };
    }

    [[nodiscard]] const std::vector<VulkanBuffer>& getBuffers() const noexcept { return uniformBuffers; }

protected:
    bool createUniformBuffers(std::string& errorMessage) {
        if (!_device) {
            errorMessage = "Failed to create Vulkan uniform buffers: device is null.";
            return false;
        }

        uniformBuffers.clear();
        uniformBuffers.reserve(_framesInFlight);

        for (uint32_t i = 0; i < _framesInFlight; i++) {
            VulkanBuffer uniformBuffer;

            TRY_deprecated(uniformBuffer.create(
                BUFFER_SIZE,
                vk::BufferUsageFlagBits::eUniformBuffer,
                VMA_MEMORY_USAGE_CPU_TO_GPU,
                _device,
                errorMessage
            ));

            TRY_deprecated(uniformBuffer.mapMemory(errorMessage));

            uniformBuffers.emplace_back(std::move(uniformBuffer));
        }

        return true;
    }

    template <typename UBOType>
    void updateMemory(const uint32_t frameIndex, UBOType ubo) const {
        std::memcpy(uniformBuffers[frameIndex].getMappedPointer(), &ubo, sizeof(ubo));
    }

    static constexpr vk::DeviceSize BUFFER_SIZE = sizeof(UniformBufferType);

    const VulkanDevice* _device = nullptr;

    uint32_t _framesInFlight = 0;

    std::vector<VulkanBuffer> uniformBuffers{};
};
