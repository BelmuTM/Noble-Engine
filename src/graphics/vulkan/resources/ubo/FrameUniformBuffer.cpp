#include "FrameUniformBuffer.h"

#include <chrono>

void FrameUniformBuffer::update(
    const uint32_t frameIndex, const VulkanSwapchain& swapchain, const Camera& camera
) {
    const vk::Extent2D swapchainExtent = swapchain.getExtent();

    static auto startTime = std::chrono::high_resolution_clock::now();

    const auto  currentTime      = std::chrono::high_resolution_clock::now();
    const float frameTimeCounter = std::chrono::duration<float>(currentTime - startTime).count();

    FrameUBO ubo{};

    ubo.view        = camera.getViewMatrix();
    ubo.viewInverse = glm::inverse(ubo.view);

    ubo.projection        = camera.getProjectionMatrix(swapchain.getAspectRatio());
    ubo.projectionInverse = glm::inverse(ubo.projection);

    ubo.projection[1][1] *= -1;

    ubo.cameraPosition = camera.getPosition();

    ubo.nearPlane = camera.getNearPlane();
    ubo.farPlane  = camera.getFarPlane();

    ubo.frameTimeCounter = frameTimeCounter;
    ubo.frameCounter     = static_cast<float>(_frameCounter);

    ubo.viewWidth  = static_cast<float>(swapchainExtent.width);
    ubo.viewHeight = static_cast<float>(swapchainExtent.height);

    ++_frameCounter;

    updateMemory(frameIndex, ubo);
}
