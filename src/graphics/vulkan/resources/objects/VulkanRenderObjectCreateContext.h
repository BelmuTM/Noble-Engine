#pragma once

#include "core/entities/objects/ObjectManager.h"

#include "graphics/vulkan/core/VulkanDevice.h"

#include "graphics/vulkan/resources/images/VulkanImageManager.h"
#include "graphics/vulkan/resources/meshes/VulkanMeshManager.h"

struct VulkanRenderObjectCreateContext {
    const ObjectManager* objectManager = nullptr;
    const AssetManager*  assetManager  = nullptr;

    const VulkanDevice* device = nullptr;

    VulkanMeshManager*  meshManager  = nullptr;
    VulkanImageManager* imageManager = nullptr;

    uint32_t framesInFlight;
};
