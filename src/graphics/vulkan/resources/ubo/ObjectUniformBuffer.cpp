#include "ObjectUniformBuffer.h"

#include <glm/gtc/matrix_transform.hpp>

void ObjectUniformBuffer::update(const uint32_t frameIndex, const glm::mat4& modelMatrix) const {
    ObjectUBO ubo{};
    ubo.model  = modelMatrix;
    ubo.normal = glm::mat4(glm::transpose(glm::inverse(glm::mat3(modelMatrix))));

    updateMemory(frameIndex, ubo);
}
