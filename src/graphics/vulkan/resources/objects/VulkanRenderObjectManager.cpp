#include "VulkanRenderObjectManager.h"

#include "core/debug/ErrorHandling.h"

bool VulkanRenderObjectManager::create(
    const std::vector<Object>& objects,
    const VulkanDevice&        device,
    VulkanDescriptorManager&   descriptorManager,
    VulkanImageManager&        imageManager,
    VulkanMeshManager&         meshManager,
    std::string&               errorMessage
) noexcept {
    _descriptorManager = &descriptorManager;
    _imageManager      = &imageManager;
    _meshManager       = &meshManager;

    _renderObjects.reserve(objects.size());
    _meshes.reserve(objects.size());

    for (uint32_t i = 0; i < objects.size(); i++) {
        VulkanRenderObject renderObject;

        TRY(createRenderObject(renderObject, i, objects[i], errorMessage));

        _meshes.push_back(renderObject.mesh.get());
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
    VulkanRenderObject& renderObject, const uint32_t objectIndex, const Object& object, std::string& errorMessage
) const {
    renderObject.objectIndex = objectIndex;
    renderObject.object      = &object;

    // Load mesh
    renderObject.mesh = std::make_unique<VulkanMesh>();
    TRY(_meshManager->loadModel(*renderObject.mesh, object.getModelPath(), errorMessage));

    // Load texture
    renderObject.texture = std::make_unique<VulkanImage>();
    if (!_imageManager->loadTextureFromFile(*renderObject.texture, renderObject.mesh->getMaterial().albedoPath, errorMessage)) {
        // Default texture fallback
        VulkanImage defaultTexture;
        TRY(_imageManager->createDefaultTexture(defaultTexture, errorMessage));
        renderObject.texture = std::make_unique<VulkanImage>(defaultTexture);
    }

    // Bind texture descriptor
    renderObject.descriptorSets = std::make_unique<VulkanDescriptorSets>(*_descriptorManager);
    TRY(renderObject.descriptorSets->allocate(errorMessage));

    renderObject.descriptorSets->bindPerFrameResource(renderObject.texture->getDescriptorInfo(0));

    return true;
}

void VulkanRenderObjectManager::updateObjects() {
    for (auto& renderObject : _renderObjects) {
        renderObject.update();

        _objectBuffer.update(renderObject.objectIndex, renderObject.data);
    }
}
