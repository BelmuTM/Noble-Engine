#pragma once

#include "VulkanObjectBuffer.h"
#include "VulkanRenderObject.h"
#include "VulkanRenderObjectCreateContext.h"

#include "graphics/vulkan/rendergraph/resources/VulkanRenderResources.h"
#include "graphics/vulkan/resources/descriptors/VulkanDescriptorInfo.h"

#include "core/resources/AssetManager.h"
#include "core/entities/objects/ObjectManager.h"

static const VulkanDescriptorScheme objectDescriptorScheme = {
    {0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment},
    {1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment},
    {2, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}
};

class VulkanRenderObjectManager {
public:
    static constexpr uint32_t MAX_RENDER_OBJECTS = 2048;

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

    void updateObjects() const;

    [[nodiscard]] const VulkanDescriptorManager& getDescriptorManager() const noexcept { return _descriptorManager; }

    [[nodiscard]] const RenderObjectsVector& getRenderObjects() noexcept { return _renderObjects; }

private:
    [[nodiscard]] bool loadObjectTextures(const AssetManager::TexturesMap& textures, std::string& errorMessage) const;

    VulkanRenderObjectCreateContext _context;

    VulkanDescriptorManager _descriptorManager{};
    VulkanObjectBuffer      _objectBuffer{};

    RenderObjectsVector _renderObjects{};
};
