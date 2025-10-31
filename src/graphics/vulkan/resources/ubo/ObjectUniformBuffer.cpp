#include "ObjectUniformBuffer.h"

#include <chrono>

#include <glm/gtc/matrix_transform.hpp>

void ObjectUniformBuffer::update(
    const uint32_t frameIndex, const VulkanSwapchain& swapchain, const Camera& camera
) const {
    static auto startTime = std::chrono::high_resolution_clock::now();

    const auto  currentTime      = std::chrono::high_resolution_clock::now();
    const float frameTimeCounter = std::chrono::duration<float>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model      = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.view       = camera.getViewMatrix();
    ubo.projection = camera.getProjectionMatrix(swapchain.getAspectRatio());

    ubo.projection[1][1] *= -1;

    memcpy(uniformBuffers[frameIndex].getMappedPointer(), &ubo, sizeof(ubo));
}
