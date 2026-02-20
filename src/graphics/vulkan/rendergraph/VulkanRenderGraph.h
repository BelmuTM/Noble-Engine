#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/core/VulkanInstance.h"
#include "graphics/vulkan/resources/VulkanFrameResources.h"

#include "nodes/VulkanRenderPass.h"
#include "resources/VulkanFrameDraws.h"

class VulkanRenderResources;

struct VulkanRenderGraphCreateContext {
    const VulkanInstance*             instance    = nullptr;
    const VulkanDevice*               device      = nullptr;
    const VulkanSwapchain*            swapchain   = nullptr;

    VulkanFrameResources*             frame     = nullptr;
    VulkanRenderResources*            resources = nullptr;

    const VulkanFrameDraws*           frameDraws = nullptr;

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

    [[nodiscard]] bool create(const VulkanRenderGraphCreateContext& context, std::string& errorMessage) noexcept;

    void destroy() noexcept;

    void execute(vk::CommandBuffer commandBuffer) const;

    bool executePass(vk::CommandBuffer commandBuffer, const VulkanRenderPass& pass, std::string& errorMessage) const;

    [[nodiscard]]       std::vector<std::unique_ptr<VulkanRenderPass>>& getPasses()       noexcept { return _passes; }
    [[nodiscard]] const std::vector<std::unique_ptr<VulkanRenderPass>>& getPasses() const noexcept { return _passes; }

    void addPass(std::unique_ptr<VulkanRenderPass> pass) {
        _passes.push_back(std::move(pass));
    }

private:
    VulkanRenderGraphCreateContext _context{};

    std::vector<std::unique_ptr<VulkanRenderPass>> _passes{};
};
