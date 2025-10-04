#pragma once
#ifndef NOBLEENGINE_VULKANUNIFORMBUFFERS_H
#define NOBLEENGINE_VULKANUNIFORMBUFFERS_H

#include "graphics/vulkan/common/VulkanHeader.h"
#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/core/memory/VulkanBuffer.h"
#include "graphics/vulkan/resources/VulkanDescriptor.h"

#include <glm/glm.hpp>

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

template<typename T>
class VulkanUniformBuffers {
public:
    VulkanUniformBuffers()  = default;
    ~VulkanUniformBuffers() = default;

    VulkanUniformBuffers(const VulkanUniformBuffers&)            = delete;
    VulkanUniformBuffers& operator=(const VulkanUniformBuffers&) = delete;
    VulkanUniformBuffers(VulkanUniformBuffers&&)                 = delete;
    VulkanUniformBuffers& operator=(VulkanUniformBuffers&&)      = delete;

    [[nodiscard]] bool create(
        const VulkanDevice& device,
        const uint32_t      framesInFlight,
        std::string&        errorMessage
    ) noexcept {
        _device         = &device;
        _framesInFlight = framesInFlight;

        if (!createUniformBuffers(errorMessage)) return false;

        return true;
    }

    void destroy() noexcept {
        for (auto& uniformBuffer : uniformBuffers) {
            uniformBuffer.destroy();
        }
        _device = nullptr;
    }

    void bindToDescriptor(const VulkanDescriptor& descriptor, const uint32_t binding) const {
        for (size_t i = 0; i < _framesInFlight; i++) {
            vk::DescriptorBufferInfo descriptorBufferInfo{};
            descriptorBufferInfo
                .setBuffer(uniformBuffers[i])
                .setOffset(0)
                .setRange(size);

            descriptor.updateSet(i, binding, vk::DescriptorType::eUniformBuffer, descriptorBufferInfo);
        }
    }

    [[nodiscard]] const std::vector<VulkanBuffer>& getBuffers() const { return uniformBuffers; }

private:
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

        for (size_t i = 0; i < _framesInFlight; i++) {
            VulkanBuffer uniformBuffer;

            if (!uniformBuffer.create(
                    size,
                    vk::BufferUsageFlagBits::eUniformBuffer,
                    VMA_MEMORY_USAGE_CPU_TO_GPU,
                    _device,
                    errorMessage
                )) {
                return false;
            }

            if (!uniformBuffer.mapMemory(errorMessage)) return false;

            uniformBuffers.emplace_back(std::move(uniformBuffer));
        }
        return true;
    }
};

#endif //NOBLEENGINE_VULKANUNIFORMBUFFERS_H