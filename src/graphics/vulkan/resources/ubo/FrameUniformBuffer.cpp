#include "FrameUniformBuffer.h"

#include <chrono>

void FrameUniformBuffer::update(
    const uint32_t frameIndex, const VulkanSwapchain& swapchain, const Camera& camera
) const {
    static auto startTime = std::chrono::high_resolution_clock::now();

    const auto  currentTime      = std::chrono::high_resolution_clock::now();
    const float frameTimeCounter = std::chrono::duration<float>(currentTime - startTime).count();

    FrameUBO ubo{};
    ubo.view       = camera.getViewMatrix();
    ubo.projection = camera.getProjectionMatrix(swapchain.getAspectRatio());

    ubo.projection[1][1] *= -1;

    ubo.frameTimeCounter = frameTimeCounter;

    updateMemory(frameIndex, ubo);
}
