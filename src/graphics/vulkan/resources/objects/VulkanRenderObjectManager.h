#pragma once
#ifndef NOBLEENGINE_VULKANRENDEROBJECTMANAGER_H
#define NOBLEENGINE_VULKANRENDEROBJECTMANAGER_H

#include "VulkanObjectBuffer.h"
#include "VulkanRenderObject.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorManager.h"
#include "graphics/vulkan/resources/descriptors/VulkanDescriptorSets.h"
#include "graphics/vulkan/resources/images/VulkanImageManager.h"
#include "graphics/vulkan/resources/meshes/VulkanMeshManager.h"

#include "core/common/Types.h"
#include "core/entities/objects/Object.h"

static constexpr int MAX_OBJECTS = 32;

class VulkanRenderObjectManager {
public:
    using render_objects_vector = std::vector<std::unique_ptr<VulkanRenderObject>>;
    using meshes_vector         = std::vector<VulkanMesh*>;

    VulkanRenderObjectManager()  = default;
    ~VulkanRenderObjectManager() = default;

    VulkanRenderObjectManager(const VulkanRenderObjectManager&)            = delete;
    VulkanRenderObjectManager& operator=(const VulkanRenderObjectManager&) = delete;

    VulkanRenderObjectManager(VulkanRenderObjectManager&&)            = delete;
    VulkanRenderObjectManager& operator=(VulkanRenderObjectManager&&) = delete;

    [[nodiscard]] bool create(
        const objects_vector&    objects,
        const VulkanDevice&      device,
        VulkanDescriptorManager& descriptorManager,
        VulkanImageManager&      imageManager,
        VulkanMeshManager&       meshManager,
        std::string&             errorMessage
    ) noexcept;

    void destroy() noexcept;

    void updateObjects() const;

    [[nodiscard]] const render_objects_vector& getRenderObjects() noexcept { return _renderObjects; }

    [[nodiscard]] const meshes_vector& getMeshes() const noexcept { return _meshes; }

private:
    VulkanDescriptorManager* _descriptorManager = nullptr;
    VulkanImageManager*      _imageManager      = nullptr;
    VulkanMeshManager*       _meshManager       = nullptr;

    render_objects_vector _renderObjects{};
    meshes_vector         _meshes{};

    VulkanObjectBuffer _objectBuffer;

    [[nodiscard]] bool createRenderObject(
        VulkanRenderObject& renderObject, uint32_t objectIndex, Object* object, std::string& errorMessage
    ) const;
};

#endif // NOBLEENGINE_VULKANRENDEROBJECTMANAGER_H
