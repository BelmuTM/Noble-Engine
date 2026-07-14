#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/core/VulkanInstance.h"
#include "graphics/vulkan/core/VulkanSwapchain.h"

#include "graphics/vulkan/resources/frame/VulkanFrameResources.h"

#include "draw/VulkanFrameCuller.h"
#include "nodes/VulkanRenderPass.h"

#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

class VulkanRenderResourceManager;

struct VulkanRenderGraphCreateContext {
    const VulkanInstance*             instance    = nullptr;
    const VulkanDevice*               device      = nullptr;
    const VulkanSwapchain*            swapchain   = nullptr;

    VulkanFrameResources*             frame               = nullptr;
    const VulkanFrameCuller*          frameCuller         = nullptr;
    const VulkanRenderObjectManager*  renderObjectManager = nullptr;

    vk::QueryPool                     queryPool;
    vk::detail::DispatchLoaderDynamic dispatchLoader{};
};

class VulkanRenderGraph {
public:
    VulkanRenderGraph()  = default;
    ~VulkanRenderGraph() = default;

    VulkanRenderGraph(const VulkanRenderGraph&)            = delete;
    VulkanRenderGraph& operator=(const VulkanRenderGraph&) = delete;

    VulkanRenderGraph(VulkanRenderGraph&&)            = delete;
    VulkanRenderGraph& operator=(VulkanRenderGraph&&) = delete;

    [[nodiscard]] Expected<void> create(const VulkanRenderGraphCreateContext& context) noexcept;

    void destroy() noexcept;

    Expected<void> execute(vk::CommandBuffer commandBuffer) const;

    Expected<void> executePass(vk::CommandBuffer commandBuffer, const VulkanRenderPass& pass) const;

    [[nodiscard]]       std::vector<std::unique_ptr<VulkanRenderPass>>& getPasses()       noexcept { return _passes; }
    [[nodiscard]] const std::vector<std::unique_ptr<VulkanRenderPass>>& getPasses() const noexcept { return _passes; }

    void addPass(std::unique_ptr<VulkanRenderPass> pass) {
        _passes.push_back(std::move(pass));
    }

private:
    VulkanRenderGraphCreateContext _context{};

    std::vector<std::unique_ptr<VulkanRenderPass>> _passes{};
};
