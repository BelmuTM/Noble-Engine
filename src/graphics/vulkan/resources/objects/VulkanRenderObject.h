#pragma once

#include "common/Utility.h"
#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/resources/meshes/VulkanRenderMesh.h"

#include "graphics/vulkan/resources/materials/VulkanMaterialManager.h"
#include "graphics/vulkan/resources/meshes/VulkanMeshManager.h"

#include "graphics/vulkan/rendergraph/draw/VulkanInstanceHandle.h"

#include "core/entities/objects/Object.h"

class VulkanRenderObject {
public:
    VulkanRenderObject()  = default;
    ~VulkanRenderObject() = default;

    VulkanRenderObject(const VulkanRenderObject&)            = delete;
    VulkanRenderObject& operator=(const VulkanRenderObject&) = delete;

    VulkanRenderObject(VulkanRenderObject&&)            noexcept = default;
    VulkanRenderObject& operator=(VulkanRenderObject&&) noexcept = default;

    Expected<void> create(
        std::uint32_t          objectIndex,
        Object*                sourceObject,
        VulkanMeshManager*     meshManager,
        VulkanMaterialManager* materialManager,
        std::uint32_t          framesInFlight
    );

    void update();

    bool sync();

    [[nodiscard]] bool isDirty(const std::uint32_t frameIndex) const noexcept {
        return (_dirtyFrames & (1 << frameIndex)) != 0;
    }

    void markDirty() noexcept {
        _dirtyFrames = _allFramesMask;
    }

    void clearDirty(const std::uint32_t frameIndex) noexcept {
        _dirtyFrames &= ~(1 << frameIndex);
    }

    [[nodiscard]] const Object& getObject() const noexcept { return *_object; }

    [[nodiscard]] const ObjectDataGPU& getGPUData() const noexcept { return _gpuData; }

    [[nodiscard]] const std::vector<VulkanRenderMesh>& getMeshes() const noexcept { return _meshes; }

    [[nodiscard]] VulkanInstanceHandle getInstanceHandle() const noexcept { return _instanceHandle; }

private:
    Object* _object = nullptr;

    std::uint32_t _syncedVersion = 0;

    ObjectDataGPU _gpuData;

    std::vector<VulkanRenderMesh> _meshes{};

    VulkanInstanceHandle _instanceHandle{};

    std::uint32_t _allFramesMask = 0;
    std::uint32_t _dirtyFrames   = 0;
};
