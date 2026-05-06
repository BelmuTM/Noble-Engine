#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/core/memory/VulkanBuffer.h"
#include "graphics/vulkan/resources/descriptors/VulkanDescriptorInfo.h"

class VulkanUniformBufferBase {
public:
    virtual ~VulkanUniformBufferBase() = default;

    [[nodiscard]] virtual Expected<void> create(const VulkanDevice& device, std::uint32_t framesInFlight) noexcept = 0;

    virtual void destroy() noexcept = 0;

    [[nodiscard]] virtual VulkanDescriptorInfo getDescriptorInfo(std::uint32_t binding, std::uint32_t frameIndex) const = 0;
};

template<typename UniformBufferType>
class VulkanUniformBuffer : public VulkanUniformBufferBase {
public:
    VulkanUniformBuffer() = default;
    ~VulkanUniformBuffer() override = default;

    VulkanUniformBuffer(const VulkanUniformBuffer&)            = delete;
    VulkanUniformBuffer& operator=(const VulkanUniformBuffer&) = delete;

    VulkanUniformBuffer(VulkanUniformBuffer&& other)            = delete;
    VulkanUniformBuffer& operator=(VulkanUniformBuffer&& other) = delete;

    [[nodiscard]] Expected<void> create(
        const VulkanDevice& device, const std::uint32_t framesInFlight
    ) noexcept override {
        _device         = &device;
        _framesInFlight = framesInFlight;

        TRY(createUniformBuffers());

        return {};
    }

    void destroy() noexcept override {
        for (auto& uniformBuffer : _uniformBuffers) {
            uniformBuffer.destroy();
        }

        _uniformBuffers.clear();

        _device = nullptr;
    }

    [[nodiscard]] VulkanDescriptorInfo getDescriptorInfo(
        const std::uint32_t binding, const std::uint32_t frameIndex
    ) const noexcept override {
        return {
            .type       = vk::DescriptorType::eUniformBuffer,
            .bufferInfo = {_uniformBuffers[frameIndex].handle(), 0, getBufferSize()},
            .binding    = binding
        };
    }

    [[nodiscard]] const std::vector<VulkanBuffer>& getBuffers() const noexcept { return _uniformBuffers; }

protected:
    Expected<void> createUniformBuffers() {
        if (!_device) {
            return VK_FAIL("Failed to create uniform buffers: device is null.");
        }

        _uniformBuffers.clear();
        _uniformBuffers.reserve(_framesInFlight);

        for (std::uint32_t i = 0; i < _framesInFlight; i++) {
            VulkanBuffer uniformBuffer;

            TRY(uniformBuffer.create(
                getBufferSize(),
                vk::BufferUsageFlagBits::eUniformBuffer,
                VMA_MEMORY_USAGE_CPU_TO_GPU,
                _device
            ));

            TRY(uniformBuffer.mapMemory());

            _uniformBuffers.emplace_back(std::move(uniformBuffer));
        }

        return {};
    }

    void updateMemory(const std::uint32_t frameIndex, const UniformBufferType& data) const {
        _uniformBuffers[frameIndex].updateMemory(data);
    }

    [[nodiscard]] vk::DeviceSize getBufferSize() const noexcept {
        return VulkanBuffer::align(
            sizeof(UniformBufferType),
            _device->getLimits().minUniformBufferOffsetAlignment
        );
    }

    const VulkanDevice* _device = nullptr;

    std::uint32_t _framesInFlight = 0;

    std::vector<VulkanBuffer> _uniformBuffers{};
};
