#pragma once

#include "graphics/vulkan/resources/meshes/VulkanRenderMesh.h"

#include "graphics/vulkan/rendergraph/draw/VulkanInstanceHandle.h"

struct VulkanRenderItem {
    VulkanRenderMesh     _renderMesh{};
    VulkanInstanceHandle _instanceHandle{};

    const glm::mat4* _modelMatrix = nullptr;
};
