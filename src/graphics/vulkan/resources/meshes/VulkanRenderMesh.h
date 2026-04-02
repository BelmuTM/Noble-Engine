#pragma once

#include "graphics/vulkan/resources/materials/VulkanMaterial.h"
#include "graphics/vulkan/resources/meshes/VulkanMesh.h"

struct VulkanRenderMesh {
    VulkanMesh*     mesh     = nullptr;
    VulkanMaterial* material = nullptr;
};
