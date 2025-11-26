#pragma once
#ifndef NOBLEENGINE_VULKANRENDERGRAPH_H
#define NOBLEENGINE_VULKANRENDERGRAPH_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/resources/VulkanFrameResources.h"
#include "graphics/vulkan/resources/meshes/VulkanMeshManager.h"

#include "nodes/VulkanRenderPass.h"

class VulkanRenderGraph {
public:
    VulkanRenderGraph()  = default;
    ~VulkanRenderGraph() = default;

    VulkanRenderGraph(const VulkanRenderGraph&)            = delete;
    VulkanRenderGraph& operator=(const VulkanRenderGraph&) = delete;

    VulkanRenderGraph(VulkanRenderGraph&&)            = delete;
    VulkanRenderGraph& operator=(VulkanRenderGraph&&) = delete;

    [[nodiscard]] bool create(
        const VulkanMeshManager& meshManager,
        VulkanFrameResources&    frame,
        VulkanRenderResources&   resources,
        vk::QueryPool            queryPool,
        std::string&             errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]]       std::vector<std::unique_ptr<VulkanRenderPass>>& getPasses()       noexcept { return _passes; }
    [[nodiscard]] const std::vector<std::unique_ptr<VulkanRenderPass>>& getPasses() const noexcept { return _passes; }

    void addPass(std::unique_ptr<VulkanRenderPass> pass) {
        _passes.push_back(std::move(pass));
    }

    void attachSwapchainOutput(const VulkanSwapchain& swapchain) const;

    void execute(vk::CommandBuffer commandBuffer) const;

    bool executePass(vk::CommandBuffer commandBuffer, const VulkanRenderPass* pass, std::string& errorMessage) const;

private:
    const VulkanMeshManager* _meshManager = nullptr;
    VulkanFrameResources*    _frame       = nullptr;
    VulkanRenderResources*   _resources   = nullptr;

    vk::QueryPool _queryPool{};

    std::vector<std::unique_ptr<VulkanRenderPass>> _passes{};
};

#endif //NOBLEENGINE_VULKANRENDERGRAPH_H
