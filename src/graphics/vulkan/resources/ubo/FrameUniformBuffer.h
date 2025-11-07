#pragma once
#ifndef NOBLEENGINE_FRAMEUNIFORMBUFFER_H
#define NOBLEENGINE_FRAMEUNIFORMBUFFER_H

#include "VulkanUniformBuffer.h"
#include "graphics/vulkan/core/VulkanSwapchain.h"

#include "core/objects/Camera.h"

#include <glm/glm.hpp>

struct FrameUBO {
    glm::mat4 view;
    glm::mat4 viewInverse;

    glm::mat4 projection;
    glm::mat4 projectionInverse;

    glm::vec3 cameraPosition;

    float frameTimeCounter;
};

class FrameUniformBuffer final : public VulkanUniformBuffer<FrameUBO> {
public:
    void update(uint32_t frameIndex, const VulkanSwapchain& swapchain, const Camera& camera) const;
};

#endif // NOBLEENGINE_FRAMEUNIFORMBUFFER_H
