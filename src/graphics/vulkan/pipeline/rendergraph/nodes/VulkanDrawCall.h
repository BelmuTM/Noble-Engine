#pragma once
#ifndef NOBLEENGINE_VULKANDRAWCALL_H
#define NOBLEENGINE_VULKANDRAWCALL_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/resources/VulkanFrameContext.h"
#include "graphics/vulkan/resources/meshes/VulkanMesh.h"

#include <functional>

struct VulkanDrawCall {
    virtual ~VulkanDrawCall() = default;

    VulkanMesh mesh{};

    std::function<std::vector<vk::DescriptorSet>(const VulkanFrameContext&)> descriptorResolver{};

    std::optional<vk::Viewport> viewport{};
    std::optional<vk::Rect2D>   scissor{};

    [[nodiscard]] vk::Viewport resolveViewport(const VulkanFrameContext& frame) const {
        if (viewport) return *viewport;
        return vk::Viewport{
            0.0f, 0.0f, static_cast<float>(frame.extent.width), static_cast<float>(frame.extent.height), 0.0f, 1.0f
        };
    }

    [[nodiscard]] vk::Rect2D resolveScissor(const VulkanFrameContext& frame) const {
        if (scissor) return *scissor;
        return {vk::Offset2D(0, 0), frame.extent};
    }

    VulkanDrawCall& setMesh(const VulkanMesh& _mesh) noexcept { mesh = _mesh; return *this; }

    VulkanDrawCall& setDescriptorResolver(
        const std::function<std::vector<vk::DescriptorSet>(const VulkanFrameContext&)>& _descriptorResolver
    ) {
        descriptorResolver = _descriptorResolver; return *this;
    }

    VulkanDrawCall& setViewport(const vk::Viewport& _viewport) noexcept { viewport = _viewport; return *this; }

    VulkanDrawCall& setScissor(const vk::Rect2D _scissor) noexcept { scissor = _scissor; return *this; }
};

struct DrawCallPushConstantBase {
    virtual ~DrawCallPushConstantBase() = default;
    virtual void pushConstants(
        vk::CommandBuffer    cmdBuffer,
        vk::PipelineLayout   layout,
        vk::ShaderStageFlags stageFlags,
        const VulkanFrameContext&  frame
    ) const = 0;
};

template<typename PushConstantType>
struct DrawCallPushConstant final : VulkanDrawCall, DrawCallPushConstantBase {
    std::function<PushConstantType(const VulkanFrameContext&)> pushConstantResolver;

    DrawCallPushConstant& setPushConstantResolver(
        const std::function<PushConstantType(const VulkanFrameContext&)>& _pushConstantResolver
    ) {
        pushConstantResolver = _pushConstantResolver; return *this;
    }

    void pushConstants(
        const vk::CommandBuffer    cmdBuffer,
        const vk::PipelineLayout   layout,
        const vk::ShaderStageFlags stageFlags,
        const VulkanFrameContext&        frame
    ) const override {
        if (!pushConstantResolver) return;

        PushConstantType data = pushConstantResolver(frame);
        cmdBuffer.pushConstants(layout, stageFlags, 0, sizeof(PushConstantType), &data);
    }
};

#endif // NOBLEENGINE_VULKANDRAWCALL_H
