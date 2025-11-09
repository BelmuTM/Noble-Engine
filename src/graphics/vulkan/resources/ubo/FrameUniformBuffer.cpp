#include "FrameUniformBuffer.h"

#include <chrono>

void FrameUniformBuffer::update(
    const uint32_t frameIndex, const VulkanSwapchain& swapchain, const Camera& camera
) const {
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

    updateMemory(frameIndex, ubo);
}
