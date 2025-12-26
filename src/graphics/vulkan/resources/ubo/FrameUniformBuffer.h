#pragma once
#ifndef NOBLEENGINE_FRAMEUNIFORMBUFFER_H
#define NOBLEENGINE_FRAMEUNIFORMBUFFER_H

#include "VulkanUniformBuffer.h"
#include "graphics/vulkan/core/VulkanSwapchain.h"

#include "core/entities/camera/Camera.h"

#include <glm/glm.hpp>

struct alignas(16) FrameUBO {
    glm::mat4 view;
    glm::mat4 viewInverse;

    glm::mat4 projection;
    glm::mat4 projectionInverse;

    glm::vec3 cameraPosition;

    float nearPlane;
    float farPlane;

    float frameTimeCounter;
    float frameCounter;

    float viewWidth;
    float viewHeight;
};

class FrameUniformBuffer final : public VulkanUniformBuffer<FrameUBO> {
public:
    void update(uint32_t frameIndex, const VulkanSwapchain& swapchain, const Camera& camera);

private:
    uint32_t _frameCounter = 0;
};

#endif // NOBLEENGINE_FRAMEUNIFORMBUFFER_H
