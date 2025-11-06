#pragma once
#ifndef NOBLEENGINE_OBJECTUNIFORMBUFFER_H
#define NOBLEENGINE_OBJECTUNIFORMBUFFER_H

#include "VulkanUniformBuffer.h"

#include <glm/glm.hpp>

struct ObjectUBO {
    glm::mat4 model;
    glm::mat4 normal;
};

class ObjectUniformBuffer final : public VulkanUniformBuffer<ObjectUBO> {
public:
    void update(uint32_t frameIndex, const glm::mat4& modelMatrix) const;
};

#endif //NOBLEENGINE_OBJECTUNIFORMBUFFER_H