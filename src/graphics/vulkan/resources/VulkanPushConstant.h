#pragma once
#ifndef NOBLEENGINE_VULKANPUSHCONSTANT_H
#define NOBLEENGINE_VULKANPUSHCONSTANT_H

#include <unordered_map>

struct VulkanPushConstantRange {
    vk::ShaderStageFlags stageFlags;
    uint32_t offset = 0;
    uint32_t size   = 0;
};

struct IVulkanPushConstant {
    virtual ~IVulkanPushConstant() = default;
    virtual void push(
        const vk::CommandBuffer&       cmdBuffer,
        const vk::PipelineLayout&      layout,
        const VulkanPushConstantRange& range
    ) const = 0;
};

template <typename PushConstantType>
struct VulkanPushConstant final : IVulkanPushConstant {
    const PushConstantType* ptr = nullptr;

    explicit VulkanPushConstant(const PushConstantType* data) : ptr(data) {}

    void push(const vk::CommandBuffer&       commandBuffer,
              const vk::PipelineLayout&      layout,
              const VulkanPushConstantRange& range
    ) const override {
        if(ptr) {
            commandBuffer.pushConstants(layout, range.stageFlags, range.offset, range.size, ptr);
        }
    }
};

using PushConstantsMap = std::unordered_map<std::string, VulkanPushConstantRange>;

#endif // NOBLEENGINE_VULKANPUSHCONSTANT_H
