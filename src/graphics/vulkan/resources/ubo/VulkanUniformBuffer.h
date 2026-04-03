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
        std::uint32_t       framesInFlight,
        std::string&        errorMessage
    ) noexcept = 0;

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

    [[nodiscard]] bool create(
        const VulkanDevice& device,
        const std::uint32_t framesInFlight,
        std::string&        errorMessage
    ) noexcept override {
        _device         = &device;
        _framesInFlight = framesInFlight;

        TRY_BOOL(createUniformBuffers(errorMessage));

        return true;
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
    bool createUniformBuffers(std::string& errorMessage) {
        if (!_device) {
            errorMessage = "Failed to create Vulkan uniform buffers: device is null.";
            return false;
        }

        _uniformBuffers.clear();
        _uniformBuffers.reserve(_framesInFlight);

        for (std::uint32_t i = 0; i < _framesInFlight; i++) {
            VulkanBuffer uniformBuffer;

            TRY_BOOL(uniformBuffer.create(
                getBufferSize(),
                vk::BufferUsageFlagBits::eUniformBuffer,
                VMA_MEMORY_USAGE_CPU_TO_GPU,
                _device,
                errorMessage
            ));

            TRY_BOOL(uniformBuffer.mapMemory(errorMessage));

            _uniformBuffers.emplace_back(std::move(uniformBuffer));
        }

        return true;
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
