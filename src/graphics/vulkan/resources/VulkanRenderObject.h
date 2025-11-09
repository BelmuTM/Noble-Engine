#pragma once
#ifndef NOBLEENGINE_VULKANRENDEROBJECT_H
#define NOBLEENGINE_VULKANRENDEROBJECT_H

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorManager.h"
#include "graphics/vulkan/resources/descriptors/VulkanDescriptorSets.h"
#include "graphics/vulkan/resources/image/VulkanImageManager.h"
#include "graphics/vulkan/resources/mesh/VulkanMeshManager.h"
#include "graphics/vulkan/resources/ubo/ObjectUniformBuffer.h"
#include "graphics/vulkan/resources/ubo/VulkanUniformBufferManager.h"

#include "core/debug/ErrorHandling.h"
#include "core/objects/Object.h"

#include <memory>

struct VulkanRenderObject {
    const Object* object;

    std::unique_ptr<VulkanMesh>  mesh;
    std::unique_ptr<VulkanImage> texture;

    std::unique_ptr<ObjectUniformBuffer> ubo;

    std::unique_ptr<VulkanDescriptorSets> descriptorSets;

    bool create(
        const Object&                  _object,
        const VulkanDescriptorManager& descriptorManager,
        VulkanImageManager&            imageManager,
        VulkanMeshManager&             meshManager,
        VulkanUniformBufferManager&    uboManager,
        std::string&                   errorMessage
    ) {
        object = &_object;

        mesh = std::make_unique<VulkanMesh>();
        TRY(meshManager.loadModel(*mesh, _object.getModelPath(), errorMessage));

        texture = std::make_unique<VulkanImage>();
        if (!imageManager.loadTextureFromFile(*texture, _object.getTexturePath(), errorMessage)) {
            VulkanImage defaultTexture;
            TRY(imageManager.createDefaultTexture(defaultTexture, errorMessage));
            texture = std::make_unique<VulkanImage>(defaultTexture);
        }

        ubo = std::make_unique<ObjectUniformBuffer>();
        TRY(uboManager.createBuffer(*ubo, errorMessage));

        descriptorSets = std::make_unique<VulkanDescriptorSets>(descriptorManager);
        TRY(descriptorSets->allocate(errorMessage));

        descriptorSets->bindPerFrameUBO(*ubo, 0);
        descriptorSets->bindPerFrameResource(texture->getDescriptorInfo(1));

        return true;
    }
};

#endif // NOBLEENGINE_VULKANRENDEROBJECT_H
