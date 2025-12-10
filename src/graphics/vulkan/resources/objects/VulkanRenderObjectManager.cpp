#include "VulkanRenderObjectManager.h"

#include "core/debug/ErrorHandling.h"
#include "core/debug/Logger.h"

bool VulkanRenderObjectManager::create(
    const ObjectsVector& objects,
    const VulkanDevice&  device,
    VulkanImageManager&  imageManager,
    VulkanMeshManager&   meshManager,
    const uint32_t       framesInFlight,
    std::string&         errorMessage
) noexcept {
    _imageManager = &imageManager;
    _meshManager  = &meshManager;

    _renderObjects.reserve(objects.size());

    TRY(_descriptorManager.create(
        device.getLogicalDevice(), objectDescriptorScheme, framesInFlight, MAX_RENDER_OBJECTS, errorMessage
    ));

    TRY(_objectBuffer.create(device, MAX_RENDER_OBJECTS, errorMessage));

    TRY(createRenderObjects(objects, errorMessage));

    return true;
}

void VulkanRenderObjectManager::destroy() noexcept {
    _descriptorManager.destroy();
    _objectBuffer.destroy();

    _imageManager = nullptr;
    _meshManager  = nullptr;
}

bool VulkanRenderObjectManager::createRenderObjects(const ObjectsVector& objects, std::string& errorMessage) {
    uint32_t meshCount = 0;

    for (uint32_t i = 0; i < objects.size(); i++) {
        if (i >= MAX_RENDER_OBJECTS) {
            Logger::warning(
                "Reached render objects capacity (" + std::to_string(MAX_RENDER_OBJECTS) +
                "), remaining objects will be skipped"
            );
            break;
        }

        auto renderObject = std::make_unique<VulkanRenderObject>();

        TRY(createRenderObject(*renderObject, i, objects[i].get(), meshCount, errorMessage));

        _renderObjects.push_back(std::move(renderObject));
    }

    return true;
}

bool VulkanRenderObjectManager::createRenderObject(
    VulkanRenderObject& renderObject,
    const uint32_t      objectIndex,
    Object*             object,
    uint32_t&           meshCount,
    std::string&        errorMessage
) {
    renderObject.objectIndex = objectIndex;
    renderObject.object      = object;

    const auto& model = object->getModel();
    renderObject.submeshes.reserve(model.meshes.size());

    for (const Mesh& mesh : model.meshes) {
        if (meshCount >= MAX_RENDER_OBJECTS) {
            Logger::warning(
                "Reached descriptor pool capacity (" + std::to_string(MAX_RENDER_OBJECTS) +
                "), remaining submeshes for object \"" + model.name + "\" will be skipped"
            );
            break;
        }

        // Load mesh
        VulkanRenderSubmesh submesh{};
        submesh.mesh = _meshManager->allocateMesh(mesh);

        // Load textures
        const Material& material = mesh.getMaterial();

        const Image* albedoTexture   = object->getTexture(material.albedoPath);
        const Image* normalTexture   = object->getTexture(material.normalPath);
        const Image* specularTexture = object->getTexture(material.specularPath);

        if (albedoTexture) {
            TRY(_imageManager->loadImage(submesh.albedoTexture, albedoTexture, true, errorMessage));
        } else {
            const Image diffuseColorImage = Image::createSinglePixelImage(material.diffuse);
            TRY(_imageManager->loadImage(submesh.albedoTexture, &diffuseColorImage, false, errorMessage));
        }

        if (normalTexture) {
            TRY(_imageManager->loadImage(submesh.normalTexture, normalTexture, true, errorMessage));
        } else {
            const Image normalImage = Image::createSinglePixelImage(glm::vec3{0.0f});
            TRY(_imageManager->loadImage(submesh.normalTexture, &normalImage, false, errorMessage));
        }

        if (specularTexture) {
            TRY(_imageManager->loadImage(submesh.specularTexture, specularTexture, true, errorMessage));
        } else {
            const Image specularImage = Image::createSinglePixelImage(material.specular);
            TRY(_imageManager->loadImage(submesh.specularTexture, &specularImage, false, errorMessage));
        }

        // Allocate and bind descriptor sets
        TRY(_descriptorManager.allocate(submesh.descriptorSets, errorMessage));
        submesh.descriptorSets->bindPerFrameResource(submesh.albedoTexture->getDescriptorInfo(0));
        submesh.descriptorSets->bindPerFrameResource(submesh.normalTexture->getDescriptorInfo(1));
        submesh.descriptorSets->bindPerFrameResource(submesh.specularTexture->getDescriptorInfo(2));

        renderObject.submeshes.push_back(submesh);
        ++meshCount;
    }

    return true;
}

void VulkanRenderObjectManager::updateObjects() const {
    std::vector<ObjectDataGPU> dataToGPU(_renderObjects.size());

    for (size_t i = 0; i < _renderObjects.size(); i++) {
        auto& renderObject = *_renderObjects[i];
        renderObject.update();

        dataToGPU[i] = renderObject.data;
    }

    _objectBuffer.update(dataToGPU);
}
