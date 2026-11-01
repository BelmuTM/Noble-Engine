#pragma once

#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"
#include "graphics/vulkan/resources/frame/VulkanFrameResources.h"
#include "graphics/vulkan/resources/ssbo/VulkanStorageBufferManager.h"

#include "VulkanRenderItem.h"

class VulkanFrameCuller {
public:
    static constexpr std::uint32_t MAX_DRAWS = 5'000'000;

    using VisibilityResult = std::vector<VulkanRenderItem>;

    VulkanFrameCuller()  = default;
    ~VulkanFrameCuller() = default;

    VulkanFrameCuller(const VulkanFrameCuller&)            = delete;
    VulkanFrameCuller& operator=(const VulkanFrameCuller&) = delete;

    VulkanFrameCuller(VulkanFrameCuller&&)            = delete;
    VulkanFrameCuller& operator=(VulkanFrameCuller&&) = delete;

    [[nodiscard]] Expected<void> create(
        const VulkanDevice&         device,
        VulkanStorageBufferManager& storageBufferManager,
        std::uint32_t               framesInFlight
    ) noexcept;

    void destroy() noexcept;

    Expected<void> cull(const VulkanRenderObjectManager& renderObjectManager, const FrameUniforms& uniforms);

    [[nodiscard]] const VisibilityResult& getVisibleItems() const noexcept { return _visibleItems; }

    [[nodiscard]] static VulkanDescriptorScheme getDescriptorScheme() noexcept {
        static const VulkanDescriptorScheme scheme = {
            {0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex}
        };
        return scheme;
    }

    [[nodiscard]] const VulkanDescriptorManager& getDescriptorManager() const noexcept { return _descriptorManager; }

    [[nodiscard]] VulkanStorageBuffer* getIndirectionBuffer() const noexcept { return _indirectionBuffer; }

    [[nodiscard]] const VulkanDescriptorSets* getDescriptorSets() const noexcept { return _indirectionDescriptors; }

private:
    VisibilityResult _visibleItems{};

    VulkanDescriptorManager _descriptorManager{};

    VulkanStorageBuffer*  _indirectionBuffer      = nullptr;
    VulkanDescriptorSets* _indirectionDescriptors = nullptr;
};
