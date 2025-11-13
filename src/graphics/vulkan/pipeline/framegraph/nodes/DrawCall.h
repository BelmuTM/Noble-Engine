#pragma once
#ifndef NOBLEENGINE_DRAWCALL_H
#define NOBLEENGINE_DRAWCALL_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/pipeline/framegraph/nodes/FrameContext.h"
#include "graphics/vulkan/resources/meshes/VulkanMesh.h"

#include <functional>

struct DrawCall {
    virtual ~DrawCall() = default;

    VulkanMesh mesh{};

    std::function<std::vector<vk::DescriptorSet>(const FrameContext&)> descriptorResolver{};

    std::optional<vk::Viewport> viewport{};
    std::optional<vk::Rect2D>   scissor{};

    [[nodiscard]] vk::Viewport resolveViewport(const FrameContext& frame) const {
        if (viewport) return *viewport;
        return vk::Viewport{
            0.0f, 0.0f, static_cast<float>(frame.extent.width), static_cast<float>(frame.extent.height), 0.0f, 1.0f
        };
    }

    [[nodiscard]] vk::Rect2D resolveScissor(const FrameContext& frame) const {
        if (scissor) return *scissor;
        return {vk::Offset2D(0, 0), frame.extent};
    }

    DrawCall& setMesh(const VulkanMesh& _mesh) { mesh = _mesh; return *this; }

    DrawCall& setDescriptorResolver(
        const std::function<std::vector<vk::DescriptorSet>(const FrameContext&)>& _descriptorResolver
    ) {
        descriptorResolver = _descriptorResolver; return *this;
    }

    DrawCall& setViewport(const vk::Viewport& _viewport) { viewport = _viewport; return *this; }

    DrawCall& setScissor(const vk::Rect2D _scissor) { scissor = _scissor; return *this; }
};

struct DrawCallPushConstantBase {
    virtual ~DrawCallPushConstantBase() = default;
    virtual void pushConstants(
        vk::CommandBuffer    cmdBuffer,
        vk::PipelineLayout   layout,
        vk::ShaderStageFlags stageFlags,
        const FrameContext&  frame
    ) const = 0;
};

template<typename PushConstantType>
struct DrawCallPushConstant final : DrawCall, DrawCallPushConstantBase {
    std::function<PushConstantType(const FrameContext&)> pushConstantResolver;

    DrawCallPushConstant& setPushConstantResolver(
        const std::function<PushConstantType(const FrameContext&)>& _pushConstantResolver
    ) {
        pushConstantResolver = _pushConstantResolver; return *this;
    }

    void pushConstants(
        const vk::CommandBuffer    cmdBuffer,
        const vk::PipelineLayout   layout,
        const vk::ShaderStageFlags stageFlags,
        const FrameContext&        frame
    ) const override {
        if (!pushConstantResolver) return;

        PushConstantType data = pushConstantResolver(frame);
        cmdBuffer.pushConstants(layout, stageFlags, 0, sizeof(PushConstantType), &data);
    }
};

#endif // NOBLEENGINE_DRAWCALL_H
