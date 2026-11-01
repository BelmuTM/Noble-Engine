#pragma once

#include "graphics/vulkan/resources/meshes/VulkanRenderMesh.h"

#include "graphics/vulkan/rendergraph/draw/VulkanInstanceHandle.h"

struct VulkanRenderItem {
    VulkanRenderMesh     renderMesh{};
    VulkanInstanceHandle instanceHandle{};
};
