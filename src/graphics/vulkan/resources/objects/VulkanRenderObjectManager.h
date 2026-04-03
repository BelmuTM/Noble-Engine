#pragma once

#include "VulkanRenderObject.h"
#include "VulkanRenderObjectCreateContext.h"

#include "graphics/vulkan/rendergraph/resources/VulkanRenderResources.h"
#include "graphics/vulkan/resources/descriptors/VulkanDescriptorInfo.h"

#include "core/entities/objects/ObjectManager.h"

class VulkanRenderObjectManager {
public:
    static constexpr std::uint32_t MAX_RENDER_OBJECTS = 2048;

    using RenderObjectsVector = std::vector<std::unique_ptr<VulkanRenderObject>>;

    VulkanRenderObjectManager()  = default;
    ~VulkanRenderObjectManager() = default;

    VulkanRenderObjectManager(const VulkanRenderObjectManager&)            = delete;
    VulkanRenderObjectManager& operator=(const VulkanRenderObjectManager&) = delete;

    VulkanRenderObjectManager(VulkanRenderObjectManager&&)            = delete;
    VulkanRenderObjectManager& operator=(VulkanRenderObjectManager&&) = delete;

    [[nodiscard]] bool create(const VulkanRenderObjectCreateContext& context, std::string& errorMessage) noexcept;

    [[nodiscard]] bool createRenderObjects(const ObjectManager::ObjectsVector& objects, std::string& errorMessage);

    void destroy() noexcept;

    void updateObjects(std::uint32_t frameIndex) const;

    [[nodiscard]] static VulkanDescriptorScheme getDescriptorScheme() noexcept {
        static const VulkanDescriptorScheme scheme = {
            {0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex},
            {1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex}
        };
        return scheme;
    }

    [[nodiscard]] const VulkanDescriptorManager& getDescriptorManager() const noexcept { return _descriptorManager; }

    [[nodiscard]] const RenderObjectsVector& getRenderObjects() const noexcept { return _renderObjects; }

private:
    VulkanRenderObjectCreateContext _context{};

    VulkanDescriptorManager _descriptorManager{};

    VulkanStorageBuffer* _objectBuffer = nullptr;

    RenderObjectsVector _renderObjects{};
};
