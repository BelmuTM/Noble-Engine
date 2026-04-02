#pragma once

#include "graphics/vulkan/resources/meshes/VulkanRenderMesh.h"

#include "graphics/vulkan/resources/images/VulkanImageManager.h"
#include "graphics/vulkan/resources/meshes/VulkanMeshManager.h"

#include "graphics/vulkan/rendergraph/draw/VulkanInstanceHandle.h"

#include "core/entities/objects/Object.h"

struct VulkanRenderObject {
    Object*       object = nullptr;
    ObjectDataGPU gpuData;

    std::vector<VulkanRenderMesh> meshes{};

    VulkanInstanceHandle instanceHandle{};

    bool create(
        const uint32_t           objectIndex,
        Object*                  sourceObject,
        VulkanMeshManager*       meshManager,
        VulkanImageManager*      imageManager,
        VulkanDescriptorManager& descriptorManager,
        std::string&             errorMessage
    ) {
        instanceHandle = VulkanInstanceHandle{objectIndex};
        object         = sourceObject;

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
        gpuData.modelMatrix  = object->getModelMatrix();
        gpuData.normalMatrix = object->getNormalMatrix();
    }
};
