#pragma once
#ifndef NOBLEENGINE_VULKANRENDEROBJECTMANAGER_H
#define NOBLEENGINE_VULKANRENDEROBJECTMANAGER_H

#include "VulkanObjectBuffer.h"
#include "VulkanRenderObject.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorManager.h"
#include "graphics/vulkan/resources/images/VulkanImageManager.h"
#include "graphics/vulkan/resources/meshes/VulkanMeshManager.h"

#include "core/common/Types.h"
#include "core/entities/objects/Object.h"

static const VulkanDescriptorManager::DescriptorScheme objectDescriptorScheme = {
    {vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}
};

class VulkanRenderObjectManager {
public:
    static constexpr int MAX_RENDER_OBJECTS = 32;

    using RenderObjectsVector = std::vector<std::unique_ptr<VulkanRenderObject>>;
    using MeshesVector         = std::vector<VulkanMesh*>;

    VulkanRenderObjectManager()  = default;
    ~VulkanRenderObjectManager() = default;

    VulkanRenderObjectManager(const VulkanRenderObjectManager&)            = delete;
    VulkanRenderObjectManager& operator=(const VulkanRenderObjectManager&) = delete;

    VulkanRenderObjectManager(VulkanRenderObjectManager&&)            = delete;
    VulkanRenderObjectManager& operator=(VulkanRenderObjectManager&&) = delete;

    [[nodiscard]] bool create(
        const ObjectsVector& objects,
        const VulkanDevice&  device,
        VulkanImageManager&  imageManager,
        VulkanMeshManager&   meshManager,
        uint32_t             framesInFlight,
        std::string&         errorMessage
    ) noexcept;

    void destroy() noexcept;

    void updateObjects() const;

    [[nodiscard]] const VulkanDescriptorManager& getDescriptorManager() const noexcept { return _descriptorManager; }

    [[nodiscard]] const RenderObjectsVector& getRenderObjects() noexcept { return _renderObjects; }

    [[nodiscard]] const MeshesVector& getMeshes() const noexcept { return _meshes; }

private:
    VulkanImageManager* _imageManager = nullptr;
    VulkanMeshManager*  _meshManager  = nullptr;

    VulkanDescriptorManager _descriptorManager{};
    VulkanObjectBuffer      _objectBuffer{};

    RenderObjectsVector _renderObjects{};
    MeshesVector        _meshes{};

    [[nodiscard]] bool createRenderObjects(const ObjectsVector& objects, std::string& errorMessage);

    [[nodiscard]] bool createRenderObject(
        VulkanRenderObject& renderObject, uint32_t objectIndex, Object* object, std::string& errorMessage
    ) const;
};

#endif // NOBLEENGINE_VULKANRENDEROBJECTMANAGER_H
