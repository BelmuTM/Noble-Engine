#include "VulkanRenderObjectManager.h"

#include "core/debug/Logger.h"

Expected<void> VulkanRenderObjectManager::create(const VulkanRenderObjectCreateContext& context) noexcept {
    _context = context;

    // Create object buffer
    TRY_ASSIGN(_objectBuffer, context.storageBufferManager->allocateBuffer(MAX_RENDER_OBJECTS * sizeof(ObjectDataGPU)));

    // Load textures

    Logger::debug("Loading object textures");

    const auto startTime = std::chrono::high_resolution_clock::now();

    TRY(context.materialManager->loadTextures(context.assetManager->getTextures()));

    const auto endTime = std::chrono::high_resolution_clock::now();

    const auto loadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    Logger::debug("Loaded object textures in " + std::to_string(loadDuration) + " ms");

    // Create render objects

    TRY(createRenderObjects(context.objectManager->getObjects()));

    return {};
}

void VulkanRenderObjectManager::destroy() noexcept {}

Expected<void> VulkanRenderObjectManager::createRenderObjects(const ObjectManager::ObjectsVector& objects) {

    _renderObjects.reserve(objects.size());

    for (std::uint32_t i = 0; i < objects.size(); i++) {

        if (i >= MAX_RENDER_OBJECTS) {
            Logger::warning(
                "Reached object buffer capacity (" + std::to_string(MAX_RENDER_OBJECTS) +
                "), skipping \"" + objects[i]->getModel().name + "\" and all following objects"
            );
            break;
        }

        _renderObjects.push_back(std::make_unique<VulkanRenderObject>());

        TRY(
            _renderObjects.back()->create(
                i, objects[i].get(), _context.meshManager, _context.materialManager, _context.framesInFlight
            )
        );
    }

    rebuildRenderItems();

    return {};
}

void VulkanRenderObjectManager::rebuildRenderItems() {
    _renderItems.clear();

    for (const auto& renderObject : _renderObjects) {
        for (const VulkanRenderMesh& renderMesh : renderObject->getMeshes()) {
            _renderItems.emplace_back(renderMesh, renderObject->getInstanceHandle());
        }
    }
}

void VulkanRenderObjectManager::updateObjects(const std::uint32_t frameIndex) const {

    for (const auto& renderObject : _renderObjects) {

        // Propagate CPU-side changes to the GPU data
        renderObject->sync();

        // Upload only if this frame's buffer is outdated
        if (!renderObject->isDirty(frameIndex)) {
            continue;
        }

        _objectBuffer->updateMemory(
            frameIndex,
            renderObject->getGPUData(),
            renderObject->getInstanceHandle().objectIndex * sizeof(ObjectDataGPU)
        );

        renderObject->clearDirty(frameIndex);
    }
}
