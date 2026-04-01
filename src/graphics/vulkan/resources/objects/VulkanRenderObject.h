#pragma once

#include "graphics/vulkan/resources/meshes/VulkanMesh.h"
#include "graphics/vulkan/resources/objects/VulkanMaterial.h"

#include "graphics/vulkan/resources/images/VulkanImageManager.h"
#include "graphics/vulkan/resources/meshes/VulkanMeshManager.h"

#include "core/entities/objects/Object.h"

struct VulkanRenderMesh {
    VulkanMesh* mesh = nullptr;

    VulkanMaterial material;
};

struct VulkanRenderObject {
    uint32_t instanceIndex = 0;

    Object* object = nullptr;

    std::vector<VulkanRenderMesh> meshes{};

    ObjectDataGPU data;

    bool create(
        const uint32_t           objectIndex,
        Object*                  sourceObject,
        VulkanMeshManager*       meshManager,
        VulkanImageManager*      imageManager,
        VulkanDescriptorManager& descriptorManager,
        std::string&             errorMessage
    ) {
        instanceIndex = objectIndex;
        object        = sourceObject;

        meshes.reserve(object->getModel().meshes.size());

        for (const Mesh& mesh : object->getModel().meshes) {
            // Load mesh
            VulkanRenderMesh submesh{};

            submesh.mesh = meshManager->allocateMesh(mesh);

            TRY_BOOL(submesh.material.create(mesh.getMaterial(), imageManager, descriptorManager, errorMessage));

            submesh.material.bindDescriptorSets();

            meshes.push_back(submesh);
        }

        return true;
    }

    void update() {
        data.modelMatrix  = object->getModelMatrix();
        data.normalMatrix = object->getNormalMatrix();
    }
};
