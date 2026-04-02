#pragma once

#include "graphics/vulkan/resources/meshes/VulkanMesh.h"
#include "graphics/vulkan/resources/objects/VulkanMaterial.h"

struct VulkanRenderMesh {
    VulkanMesh* mesh = nullptr;

    VulkanMaterial material;
};
