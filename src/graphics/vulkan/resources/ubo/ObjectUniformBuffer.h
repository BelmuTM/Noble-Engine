#pragma once
#ifndef NOBLEENGINE_OBJECTUNIFORMBUFFER_H
#define NOBLEENGINE_OBJECTUNIFORMBUFFER_H

#include "VulkanUniformBuffer.h"

#include <glm/glm.hpp>

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

class ObjectUniformBuffer final : public VulkanUniformBuffer<UniformBufferObject> {
public:
    void setSwapchain(const VulkanSwapchain& swapchain) {
        _swapchain = &swapchain;
    }

    void update(uint32_t frameIndex) override;

private:
    const VulkanSwapchain* _swapchain = nullptr;
};

#endif //NOBLEENGINE_OBJECTUNIFORMBUFFER_H