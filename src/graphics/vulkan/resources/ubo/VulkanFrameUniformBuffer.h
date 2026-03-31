#pragma once

#include "VulkanUniformBuffer.h"

#include "core/render/FrameUniforms.h"

class VulkanFrameUniformBuffer final : public VulkanUniformBuffer<FrameUniforms> {
public:
    void update(uint32_t frameIndex, const FrameUniforms& uniforms) const;
};
