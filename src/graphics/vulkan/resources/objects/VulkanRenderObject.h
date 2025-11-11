#pragma once
#ifndef NOBLEENGINE_VULKANRENDEROBJECT_H
#define NOBLEENGINE_VULKANRENDEROBJECT_H

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorSets.h"
#include "graphics/vulkan/resources/image/VulkanImage.h"
#include "graphics/vulkan/resources/mesh/VulkanMesh.h"

#include "core/objects/object/Object.h"

#include <memory>

#include <glm/glm.hpp>

struct VulkanRenderObject {
    const Object* object = nullptr;

    std::unique_ptr<VulkanMesh>  mesh;
    std::unique_ptr<VulkanImage> texture;

    std::unique_ptr<VulkanDescriptorSets> descriptorSets;

    ObjectDataGPU data{};
    uint32_t objectIndex = 0;

    void update() {
        data.model  = object->getModelMatrix();
        data.normal = glm::mat4(glm::transpose(glm::inverse(glm::mat3(data.model))));
    }
};

#endif // NOBLEENGINE_VULKANRENDEROBJECT_H
