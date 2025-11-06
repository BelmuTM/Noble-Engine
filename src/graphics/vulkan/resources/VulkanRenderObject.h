#pragma once
#ifndef NOBLEENGINE_VULKANRENDEROBJECT_H
#define NOBLEENGINE_VULKANRENDEROBJECT_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorManager.h"
#include "graphics/vulkan/resources/descriptors/VulkanDescriptorSets.h"
#include "graphics/vulkan/resources/image/VulkanImageManager.h"
#include "graphics/vulkan/resources/mesh/VulkanMeshManager.h"
#include "graphics/vulkan/resources/ubo/ObjectUniformBuffer.h"

#include "core/debug/ErrorHandling.h"
#include "core/objects/Object.h"

#include <memory>

#include "core/Engine.h"
#include "ubo/VulkanUniformBufferManager.h"

struct VulkanRenderObject {
    std::unique_ptr<VulkanMesh>  mesh;
    std::unique_ptr<VulkanImage> texture;

    std::unique_ptr<ObjectUniformBuffer> ubo;

    std::unique_ptr<VulkanDescriptorSets> descriptorSets;

    bool create(
        const Object&                  object,
        const VulkanDescriptorManager& descriptorManager,
        VulkanImageManager&            imageManager,
        const VulkanMeshManager&       meshManager,
        VulkanUniformBufferManager&    uboManager,
        std::string&                   errorMessage
    ) {
        mesh = std::make_unique<VulkanMesh>();
        TRY(meshManager.loadModel(*mesh, object.getModelPath(), errorMessage));

        texture = std::make_unique<VulkanImage>();
        TRY(imageManager.loadTextureFromFile(*texture, object.getTexturePath(), errorMessage));

        ubo = std::make_unique<ObjectUniformBuffer>();
        TRY(uboManager.createBuffer(*ubo, errorMessage));

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            ubo->update(i, object.getModelMatrix());

        descriptorSets = std::make_unique<VulkanDescriptorSets>(descriptorManager);
        TRY(descriptorSets->allocate(errorMessage));

        descriptorSets->bindPerFrameUBO(*ubo, 0);
        descriptorSets->bindPerFrameResource(texture->getDescriptorInfo(1));

        return true;
    }
};

#endif // NOBLEENGINE_VULKANRENDEROBJECT_H
