#pragma once

#include "graphics/vulkan/resources/ubo/VulkanUniformBuffer.h"

#include "core/render/FrameUniforms.h"

class VulkanFrameUniformBuffer final : public VulkanUniformBuffer<FrameUniforms> {
public:
    void update(std::uint32_t frameIndex, const FrameUniforms& uniforms) const;
};
