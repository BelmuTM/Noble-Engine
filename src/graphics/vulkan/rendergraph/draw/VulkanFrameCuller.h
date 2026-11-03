#pragma once

#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

#include "VulkanRenderItem.h"

struct FrameUniforms;

class VulkanFrameCuller {
public:
    using VisibilityResult = std::vector<VulkanRenderItem>;

    VulkanFrameCuller()  = default;
    ~VulkanFrameCuller() = default;

    VulkanFrameCuller(const VulkanFrameCuller&)            = delete;
    VulkanFrameCuller& operator=(const VulkanFrameCuller&) = delete;

    VulkanFrameCuller(VulkanFrameCuller&&)            = delete;
    VulkanFrameCuller& operator=(VulkanFrameCuller&&) = delete;

    void cull(const VulkanRenderObjectManager& renderObjectManager, const FrameUniforms& uniforms);

    [[nodiscard]] const VisibilityResult& getVisibleItems() const noexcept { return _visibleItems; }

private:
    VisibilityResult _visibleItems{};
};
