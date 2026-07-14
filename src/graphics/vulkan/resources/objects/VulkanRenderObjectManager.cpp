#include "VulkanRenderObjectManager.h"

#include "core/debug/Logger.h"

Expected<void> VulkanRenderObjectManager::create(const VulkanRenderObjectCreateContext& context) noexcept {
    _context = context;

    // Create descriptor manager
    TRY(_descriptorManager.create(
        context.device->getLogicalDevice(), getDescriptorScheme(), context.framesInFlight, MAX_RENDER_OBJECTS
    ));

    // Create object buffer
    TRY_ASSIGN(
        _objectBuffer,
        context.storageBufferManager->allocateBuffer(MAX_RENDER_OBJECTS * sizeof(ObjectDataGPU))
    );

    TRY(_descriptorManager.allocate(_objectDescriptors));

    _objectDescriptors->updatePerFrameSSBODescriptorSets(*_objectBuffer, 0);

    // Load textures

    Logger::debug("Loading object textures");

    auto startTime = std::chrono::high_resolution_clock::now();

    TRY(context.materialManager->loadTextures(context.assetManager->getTextures()));

    auto endTime = std::chrono::high_resolution_clock::now();

    auto loadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    Logger::debug("Loaded object textures in " + std::to_string(loadDuration) + " ms");

    // Create render objects

    TRY(createRenderObjects(context.objectManager->getObjects()));

    return {};
}

void VulkanRenderObjectManager::destroy() noexcept {
    _descriptorManager.destroy();
}

Expected<void> VulkanRenderObjectManager::createRenderObjects(const ObjectManager::ObjectsVector& objects) {
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

        _renderObjects.push_back(std::make_unique<VulkanRenderObject>());

        TRY(_renderObjects.back()->create(i, objects[i].get(), _context.meshManager, _context.materialManager));
    }

    return {};
}

void VulkanRenderObjectManager::updateObjects(const std::uint32_t frameIndex) const {
    std::vector<ObjectDataGPU> dataToGPU(_renderObjects.size());

    // Batched object data update
    for (std::size_t i = 0; i < _renderObjects.size(); i++) {
        VulkanRenderObject& renderObject = *_renderObjects[i];

        renderObject.update();

        dataToGPU[i] = renderObject.gpuData;
    }

    _objectBuffer->updateArrayMemory(frameIndex, dataToGPU);
}
