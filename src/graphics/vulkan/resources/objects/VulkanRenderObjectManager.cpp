#include "VulkanRenderObjectManager.h"

#include "core/debug/ErrorHandling.h"
#include "core/debug/Logger.h"

bool VulkanRenderObjectManager::create(
    const ObjectManager& objectManager,
    const AssetManager&  assetManager,
    const VulkanDevice&  device,
    VulkanImageManager&  imageManager,
    VulkanMeshManager&   meshManager,
    const uint32_t       framesInFlight,
    std::string&         errorMessage
) noexcept {
    _imageManager = &imageManager;
    _meshManager  = &meshManager;

    TRY_deprecated(_descriptorManager.create(
        device.getLogicalDevice(), objectDescriptorScheme, framesInFlight, MAX_RENDER_OBJECTS, errorMessage
    ));

    TRY_deprecated(_objectBuffer.create(device, MAX_RENDER_OBJECTS, errorMessage));

    Logger::debug("Loading object textures");

    const auto startTime = std::chrono::high_resolution_clock::now();

    TRY_deprecated(loadObjectTextures(assetManager.getTextures(), errorMessage));

    const auto endTime = std::chrono::high_resolution_clock::now();

    const auto loadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    Logger::debug("Loaded object textures in " + std::to_string(loadDuration) + " ms");

    TRY_deprecated(createRenderObjects(objectManager.getObjects(), errorMessage));

    return true;
}

void VulkanRenderObjectManager::destroy() noexcept {
    _descriptorManager.destroy();
    _objectBuffer.destroy();

    _imageManager = nullptr;
    _meshManager  = nullptr;
}

bool VulkanRenderObjectManager::loadObjectTextures(
    const AssetManager::TexturesMap& textures, std::string& errorMessage
) const {
    std::vector<const Image*> images{};

    for (const auto& texture : textures | std::views::values) {
        if (!texture) continue;
        images.push_back(texture);
    }

    TRY_deprecated(_imageManager->loadBatchedImages(images, errorMessage));

    return true;
}

bool VulkanRenderObjectManager::createRenderObjects(
    const ObjectManager::ObjectsVector& objects, std::string& errorMessage
) {
    _renderObjects.reserve(objects.size());

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

        TRY_deprecated(createRenderObject(*renderObject, i, objects[i].get(), meshCount, errorMessage));

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

        submesh.albedoTexture   = _imageManager->getImage(material.albedoPath);
        submesh.normalTexture   = _imageManager->getImage(material.normalPath);
        submesh.specularTexture = _imageManager->getImage(material.specularPath);

        if (!submesh.albedoTexture) {
            const Image diffuseColorImage = Image::createSinglePixelImage(glm::vec3{1.0f});
            TRY_deprecated(_imageManager->loadImage(submesh.albedoTexture, &diffuseColorImage, errorMessage));
        }

        if (!submesh.normalTexture) {
            const Image normalImage = Image::createSinglePixelImage(glm::vec3{0.0f});
            TRY_deprecated(_imageManager->loadImage(submesh.normalTexture, &normalImage, errorMessage));
        }

        if (!submesh.specularTexture) {
            const Image specularImage = Image::createSinglePixelImage(material.specular);
            TRY_deprecated(_imageManager->loadImage(submesh.specularTexture, &specularImage, errorMessage));
        }

        // Allocate and bind descriptor sets
        TRY_deprecated(_descriptorManager.allocate(submesh.descriptorSets, errorMessage));

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
