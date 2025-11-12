#include "VulkanRenderObjectManager.h"

#include "core/debug/ErrorHandling.h"
#include "core/debug/Logger.h"

bool VulkanRenderObjectManager::create(
    const objects_vector&    objects,
    const VulkanDevice&      device,
    VulkanDescriptorManager& descriptorManager,
    VulkanImageManager&      imageManager,
    VulkanMeshManager&       meshManager,
    std::string&             errorMessage
) noexcept {
    _descriptorManager = &descriptorManager;
    _imageManager      = &imageManager;
    _meshManager       = &meshManager;

    _renderObjects.reserve(objects.size());
    _meshes.reserve(objects.size());

    for (uint32_t i = 0; i < objects.size(); i++) {
        auto renderObject = std::make_unique<VulkanRenderObject>();

        TRY(createRenderObject(*renderObject, i, objects[i].get(), errorMessage));

        for (auto& submesh : renderObject->submeshes) {
            _meshes.push_back(submesh.mesh.get());
        }

        _renderObjects.push_back(std::move(renderObject));
    }

    TRY(_objectBuffer.create(device, MAX_OBJECTS, errorMessage));

    return true;
}

void VulkanRenderObjectManager::destroy() noexcept {
    _descriptorManager = nullptr;
    _imageManager      = nullptr;
    _meshManager       = nullptr;

    _objectBuffer.destroy();
}

bool VulkanRenderObjectManager::createRenderObject(
    VulkanRenderObject& renderObject, const uint32_t objectIndex, Object* object, std::string& errorMessage
) const {
    renderObject.objectIndex = objectIndex;
    renderObject.object      = object;

    const auto& [name, meshes] = object->getModel();
    renderObject.submeshes.reserve(meshes.size());

    for (const Mesh& mesh : meshes) {
        VulkanRenderSubmesh submesh;

        // Load mesh
        submesh.mesh = std::make_unique<VulkanMesh>(mesh);

        // Load texture
        const Material& material = submesh.mesh->getMaterial();

        submesh.texture = std::make_unique<VulkanImage>();

        const Image* albedoTexture = object->getTexture(material.albedoPath);
        if (albedoTexture) {
            TRY(_imageManager->loadImage(*submesh.texture, albedoTexture, errorMessage));
        } else {
            std::vector<uint8_t> defaultColorBytes = Image::rgbColorToBytes(material.diffuse);
            TRY(_imageManager->loadImage(*submesh.texture, defaultColorBytes.data(), errorMessage));
        }

        // Allocate and bind descriptor sets
        submesh.descriptorSets = std::make_unique<VulkanDescriptorSets>(*_descriptorManager);
        TRY(submesh.descriptorSets->allocate(errorMessage));
        submesh.descriptorSets->bindPerFrameResource(submesh.texture->getDescriptorInfo(0));

        renderObject.submeshes.push_back(std::move(submesh));
    }

    return true;
}

void VulkanRenderObjectManager::updateObjects() const {
    std::vector<ObjectDataGPU> dataToGPU(_renderObjects.size());

    for (size_t i = 0; i < _renderObjects.size(); ++i) {
        auto& renderObject = *_renderObjects[i];
        renderObject.update();

        dataToGPU[i] = renderObject.data;
    }

    _objectBuffer.update(dataToGPU);
}
