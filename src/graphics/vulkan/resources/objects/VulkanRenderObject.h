#pragma once
#ifndef NOBLEENGINE_VULKANRENDEROBJECT_H
#define NOBLEENGINE_VULKANRENDEROBJECT_H

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorSets.h"
#include "graphics/vulkan/resources/images/VulkanImage.h"
#include "graphics/vulkan/resources/meshes/VulkanMesh.h"

#include "core/entities/objects/Object.h"

#include <memory>

struct VulkanRenderSubmesh {
    VulkanMesh*           mesh;
    VulkanImage*          texture;
    VulkanDescriptorSets* descriptorSets;
};

struct VulkanRenderObject {
    Object* object = nullptr;

    std::vector<VulkanRenderSubmesh> submeshes{};

    ObjectDataGPU data{};
    uint32_t objectIndex = 0;

    void update() {
        data.model  = object->getModelMatrix();
        data.normal = object->getNormalMatrix();
    }
};

#endif // NOBLEENGINE_VULKANRENDEROBJECT_H
