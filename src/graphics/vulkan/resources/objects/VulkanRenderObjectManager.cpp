#include "VulkanRenderObjectManager.h"

#include "core/debug/ErrorHandling.h"
#include "core/debug/Logger.h"

bool VulkanRenderObjectManager::create(
    const VulkanRenderObjectCreateContext& context, std::string& errorMessage
) noexcept {
    _context = context;

    TRY_BOOL(_descriptorManager.create(
        context.device->getLogicalDevice(), getDescriptorScheme(), context.framesInFlight, MAX_RENDER_OBJECTS, errorMessage
    ));

    TRY_BOOL(_objectBuffer.create(*context.device, MAX_RENDER_OBJECTS, errorMessage));

    Logger::debug("Loading object textures");

    auto startTime = std::chrono::high_resolution_clock::now();

    TRY_BOOL(context.materialManager->loadTextures(context.assetManager->getTextures(), errorMessage));

    auto endTime = std::chrono::high_resolution_clock::now();

    auto loadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    Logger::debug("Loaded object textures in " + std::to_string(loadDuration) + " ms");

    Logger::debug("Creating objects");

    startTime = std::chrono::high_resolution_clock::now();

    TRY_BOOL(createRenderObjects(context.objectManager->getObjects(), errorMessage));

    endTime = std::chrono::high_resolution_clock::now();

    loadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    Logger::debug("Created objects in " + std::to_string(loadDuration) + " ms");

    return true;
}

void VulkanRenderObjectManager::destroy() noexcept {
    _descriptorManager.destroy();
    _objectBuffer.destroy();
}

bool VulkanRenderObjectManager::createRenderObjects(
    const ObjectManager::ObjectsVector& objects, std::string& errorMessage
) {
    _renderObjects.reserve(objects.size());

    std::uint32_t meshCount = 0;

    for (std::uint32_t i = 0; i < objects.size(); i++) {
        meshCount += objects[i]->getModel().meshes.size();

        if (meshCount >= MAX_RENDER_OBJECTS) {
            Logger::warning(
                "Reached descriptor pool capacity (" + std::to_string(MAX_RENDER_OBJECTS) +
                "), remaining submeshes for object \"" + objects[i]->getModel().name + "\" will be skipped"
            );
            break;
        }

        _renderObjects.emplace_back(std::make_unique<VulkanRenderObject>());

        TRY_BOOL(_renderObjects.back()->create(
            i, objects[i].get(), _context.meshManager, _context.materialManager, errorMessage
        ));
    }

    return true;
}

void VulkanRenderObjectManager::updateObjects() const {
    std::vector<ObjectDataGPU> dataToGPU(_renderObjects.size());

    for (std::size_t i = 0; i < _renderObjects.size(); i++) {
        auto& renderObject = *_renderObjects[i];

        renderObject.update();

        dataToGPU[i] = renderObject.gpuData;
    }

    _objectBuffer.update(dataToGPU);
}
