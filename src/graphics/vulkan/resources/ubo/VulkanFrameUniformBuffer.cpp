#include "VulkanFrameUniformBuffer.h"

void VulkanFrameUniformBuffer::update(const uint32_t frameIndex, const FrameUniforms& uniforms) const {
    updateMemory(frameIndex, uniforms);
}
