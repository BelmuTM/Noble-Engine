#include "ObjectUniformBuffer.h"

#include <chrono>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

void ObjectUniformBuffer::update(const uint32_t frameIndex) {
    assert(_swapchain != nullptr);

    static auto startTime = std::chrono::high_resolution_clock::now();

    const auto  currentTime      = std::chrono::high_resolution_clock::now();
    const float frameTimeCounter = std::chrono::duration<float>(currentTime - startTime).count();

    const vk::Extent2D& extent      = _swapchain->getExtent2D();
    const float         aspectRatio = static_cast<float>(extent.width) / static_cast<float>(extent.height);

    UniformBufferObject ubo{};
    ubo.model      = glm::rotate(glm::mat4(1.0f), frameTimeCounter * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view       = glm::lookAt(glm::vec3(2.0f, 2.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.projection = glm::perspective(glm::radians(30.0f), aspectRatio, 0.1f, 10.0f);

    ubo.projection[1][1] *= -1;

    memcpy(uniformBuffers[frameIndex].getMappedPointer(), &ubo, sizeof(ubo));
}
