#pragma once
#ifndef NOBLEENGINE_OBJECTUNIFORMBUFFER_H
#define NOBLEENGINE_OBJECTUNIFORMBUFFER_H

#include "VulkanUniformBuffer.h"

#include "core/Camera.h"

#include <glm/glm.hpp>

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

class ObjectUniformBuffer final : public VulkanUniformBuffer<UniformBufferObject> {
public:
    void update(uint32_t frameIndex, const VulkanSwapchain& swapchain, const Camera& camera) const;
};

#endif //NOBLEENGINE_OBJECTUNIFORMBUFFER_H