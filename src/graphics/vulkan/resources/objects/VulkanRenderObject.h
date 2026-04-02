#pragma once

#include "core/debug/Logger.h"
#include "graphics/vulkan/resources/meshes/VulkanRenderMesh.h"

#include "graphics/vulkan/resources/materials/VulkanMaterialManager.h"
#include "graphics/vulkan/resources/meshes/VulkanMeshManager.h"

#include "graphics/vulkan/rendergraph/draw/VulkanInstanceHandle.h"

#include "core/entities/objects/Object.h"

struct VulkanRenderObject {
    Object*       object = nullptr;
    ObjectDataGPU gpuData;

    std::vector<VulkanRenderMesh> meshes{};

    VulkanInstanceHandle instanceHandle{};

    bool create(
        const std::uint32_t    objectIndex,
        Object*                sourceObject,
        VulkanMeshManager*     meshManager,
        VulkanMaterialManager* materialManager,
        std::string&           errorMessage
    ) {
        instanceHandle = VulkanInstanceHandle{objectIndex};
        object         = sourceObject;

        meshes.reserve(object->getModel().meshes.size());

        for (const Mesh& mesh : object->getModel().meshes) {
            // Load mesh
            VulkanRenderMesh submesh{};

            submesh.mesh = meshManager->allocateMesh(mesh);

            // Load material
            VulkanMaterial* material = materialManager->getOrCreateMaterial(mesh.getMaterial(), errorMessage);
            TRY_BOOL(material);

            submesh.material = material;
            submesh.material->bindDescriptorSets();

            meshes.push_back(submesh);
        }

        return true;
    }

    void update() {
        gpuData.modelMatrix  = object->getModelMatrix();
        gpuData.normalMatrix = object->getNormalMatrix();
    }
};
