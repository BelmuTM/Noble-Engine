#pragma once

#include "VulkanRenderObject.h"
#include "VulkanRenderObjectCreateContext.h"

#include "graphics/vulkan/rendergraph/draw/VulkanRenderItem.h"

#include "core/entities/objects/ObjectManager.h"

class VulkanRenderObjectManager {
public:
    static constexpr std::uint32_t MAX_RENDER_OBJECTS = 2048;

    using RenderObjectsVector = std::vector<std::unique_ptr<VulkanRenderObject>>;
    using RenderItemsVector   = std::vector<VulkanRenderItem>;

    VulkanRenderObjectManager()  = default;
    ~VulkanRenderObjectManager() = default;

    VulkanRenderObjectManager(const VulkanRenderObjectManager&)            = delete;
    VulkanRenderObjectManager& operator=(const VulkanRenderObjectManager&) = delete;

    VulkanRenderObjectManager(VulkanRenderObjectManager&&)            = delete;
    VulkanRenderObjectManager& operator=(VulkanRenderObjectManager&&) = delete;

    [[nodiscard]] Expected<void> create(const VulkanRenderObjectCreateContext& context) noexcept;

    void destroy() noexcept;

    [[nodiscard]] Expected<void> createRenderObjects(const ObjectManager::ObjectsVector& objects);

    void rebuildRenderItems();

    void updateObjects(std::uint32_t frameIndex) const;

    [[nodiscard]] const RenderObjectsVector& getRenderObjects() const noexcept { return _renderObjects; }

    [[nodiscard]] const RenderItemsVector& getRenderItems() const noexcept { return _renderItems; }

    [[nodiscard]] const VulkanStorageBuffer* getObjectBuffer() const noexcept { return _objectBuffer; }

private:
    VulkanRenderObjectCreateContext _context{};

    RenderObjectsVector _renderObjects{};
    RenderItemsVector   _renderItems{};

    VulkanStorageBuffer* _objectBuffer = nullptr;
};
