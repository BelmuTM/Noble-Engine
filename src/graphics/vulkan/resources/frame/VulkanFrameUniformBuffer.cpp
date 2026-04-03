#include "VulkanFrameUniformBuffer.h"

void VulkanFrameUniformBuffer::update(const std::uint32_t frameIndex, const FrameUniforms& uniforms) const {
    updateMemory(frameIndex, uniforms);
}
