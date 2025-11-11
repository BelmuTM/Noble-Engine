#pragma once
#ifndef NOBLEENGINE_VULKANRENDEROBJECTMANAGER_H
#define NOBLEENGINE_VULKANRENDEROBJECTMANAGER_H

#include "VulkanRenderObject.h"
#include "VulkanObjectBuffer.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorManager.h"
#include "graphics/vulkan/resources/descriptors/VulkanDescriptorSets.h"
#include "graphics/vulkan/resources/image/VulkanImageManager.h"
#include "graphics/vulkan/resources/mesh/VulkanMeshManager.h"

#include "core/objects/object/Object.h"

static constexpr int MAX_OBJECTS = 32;

class VulkanRenderObjectManager {
public:
    VulkanRenderObjectManager()  = default;
    ~VulkanRenderObjectManager() = default;

    VulkanRenderObjectManager(const VulkanRenderObjectManager&)            = delete;
    VulkanRenderObjectManager& operator=(const VulkanRenderObjectManager&) = delete;

    VulkanRenderObjectManager(VulkanRenderObjectManager&&)            = delete;
    VulkanRenderObjectManager& operator=(VulkanRenderObjectManager&&) = delete;

    [[nodiscard]] bool create(
        const std::vector<Object>& objects,
        const VulkanDevice&        device,
        VulkanDescriptorManager&   descriptorManager,
        VulkanImageManager&        imageManager,
        VulkanMeshManager&         meshManager,
        std::string&               errorMessage
    ) noexcept;

    void destroy() noexcept;

    void updateObjects();

    [[nodiscard]] std::vector<VulkanRenderObject>& getRenderObjects() noexcept { return _renderObjects; }

    [[nodiscard]] std::vector<VulkanMesh*> getMeshes() const noexcept { return _meshes; }

private:
    VulkanDescriptorManager* _descriptorManager = nullptr;
    VulkanImageManager*      _imageManager      = nullptr;
    VulkanMeshManager*       _meshManager       = nullptr;

    std::vector<VulkanRenderObject> _renderObjects{};
    std::vector<VulkanMesh*>        _meshes{};

    VulkanObjectBuffer _objectBuffer;

    [[nodiscard]] bool createRenderObject(
        VulkanRenderObject& renderObject, uint32_t objectIndex, const Object& object, std::string& errorMessage
    ) const;
};

#endif // NOBLEENGINE_VULKANRENDEROBJECTMANAGER_H
