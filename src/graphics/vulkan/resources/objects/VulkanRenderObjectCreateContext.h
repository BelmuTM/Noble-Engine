#pragma once

#include "core/entities/objects/ObjectManager.h"

#include "graphics/vulkan/core/VulkanDevice.h"

#include "graphics/vulkan/resources/materials/VulkanMaterialManager.h"
#include "graphics/vulkan/resources/meshes/VulkanMeshManager.h"

struct VulkanRenderObjectCreateContext {
    const ObjectManager* objectManager = nullptr;
    const AssetManager*  assetManager  = nullptr;

    const VulkanDevice* device = nullptr;

    VulkanMeshManager*     meshManager     = nullptr;
    VulkanMaterialManager* materialManager = nullptr;

    std::uint32_t framesInFlight;
};
