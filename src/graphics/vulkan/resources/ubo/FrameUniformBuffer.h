#pragma once
#ifndef NOBLEENGINE_FRAMEUNIFORMBUFFER_H
#define NOBLEENGINE_FRAMEUNIFORMBUFFER_H

#include "VulkanUniformBuffer.h"
#include "graphics/vulkan/core/VulkanSwapchain.h"

#include "core/entities/camera/Camera.h"
#include "core/render/FrameUniforms.h"

class FrameUniformBuffer final : public VulkanUniformBuffer<FrameUniforms> {
public:
    void update(uint32_t frameIndex, const VulkanSwapchain& swapchain, const Camera& camera);

    [[nodiscard]] const FrameUniforms& getUniforms() const noexcept { return _uniforms; }

private:
    FrameUniforms _uniforms{};
};

#endif // NOBLEENGINE_FRAMEUNIFORMBUFFER_H
