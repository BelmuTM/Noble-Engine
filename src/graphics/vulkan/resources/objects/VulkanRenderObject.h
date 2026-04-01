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
    Object* object = nullptr;

    std::vector<VulkanRenderMesh> meshes{};

    ObjectDataGPU data;
    uint32_t objectIndex = 0;

    bool create(
        const uint32_t           _objectIndex,
        Object*                  _object,
        VulkanMeshManager*       meshManager,
        VulkanImageManager*      imageManager,
        VulkanDescriptorManager& descriptorManager,
        std::string&             errorMessage
    ) {
        objectIndex = _objectIndex;
        object      = _object;

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
