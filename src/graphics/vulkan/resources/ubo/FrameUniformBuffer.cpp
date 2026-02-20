#include "FrameUniformBuffer.h"

void FrameUniformBuffer::update(
    const uint32_t         frameIndex,
    const VulkanSwapchain& swapchain,
    const Camera&          camera,
    const DebugState&      debugState
) {
    const vk::Extent2D swapchainExtent = swapchain.getExtent();

    _uniforms.update(camera, swapchainExtent.width, swapchainExtent.height);

    _uniforms.debugMode = debugState.debugMode;

    updateMemory(frameIndex, _uniforms);
}
